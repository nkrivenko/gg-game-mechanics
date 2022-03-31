#include <nft.hpp>

/**
 * @brief Crafts a new nft
 * 
 */
ACTION nft::craft( const name& asset_name, const name& asset_owner ) {
    require_auth( _self );

    atomicdata::ATTRIBUTE_MAP mdata = {
        {"used", 0u},
        {"last_usage", 0u}
    };

    tools_t tools( _self, _self.value );
    auto asset_data = tools.find( asset_name.value );

    check( asset_data != tools.end(), "No asset with given name" );

    sub_balances( asset_owner, asset_data->craft_resources );
    consume_energy( asset_owner, asset_data->craft_energy_consumption );

    action(
        { _self, ACTIVE_PERMISSION }, atomicassets::ATOMICASSETS_ACCOUNT, "mintasset"_n,
        std::tuple(
            _self, asset_data->collection_name, asset_data->schema_name, asset_data->template_id,
            asset_owner, atomicdata::ATTRIBUTE_MAP(), mdata, std::vector<asset>()
        )
    ).send();
}

ACTION nft::repair( const name& asset_owner, const uint64_t asset_id ) {
    require_auth( _self );

    auto [ deserialized_idata, deserialized_mdata ] = get_asset_data( _self, asset_id );

    auto current_time = current_time_point().sec_since_epoch();
    check_if_not_broken(deserialized_idata, deserialized_mdata);
    check_if_on_cooldown(deserialized_idata, deserialized_mdata, current_time);

    deserialized_mdata["used"] = 0u;
    deserialized_mdata["last_usage"] = 0u;

    tools_t tools( _self, _self.value );
    auto asset_data = tools.find( name(std::get<std::string>(deserialized_idata["code"])).value );

    check( asset_data != tools.end(), "No asset staked with given name" );

    sub_balances( asset_owner, asset_data->repair_resources );
    consume_energy( asset_owner, asset_data->repair_energy_consumption );

    action(
        { _self, ACTIVE_PERMISSION }, atomicassets::ATOMICASSETS_ACCOUNT, "setassetdata"_n,
        std::tuple(_self, _self, asset_id, deserialized_mdata)
    ).send();
}

ACTION nft::mine( const name& asset_owner, const uint64_t asset_id, const std::string& memo ) {
    require_auth( _self );

    auto [ deserialized_idata, deserialized_mdata ] = get_asset_data(_self, asset_id);

    auto current_time = current_time_point().sec_since_epoch();

    check_if_broken(deserialized_idata, deserialized_mdata);
    check_if_on_cooldown(deserialized_idata, deserialized_mdata, current_time);

    cards_t assets( _self, _self.value );
    auto owner = assets.find( asset_id );
    check( owner != assets.end(), "No owner" );
    check( owner->asset_owner == asset_owner, "Wrong asset owner" );

    consume_energy( asset_owner, std::get<uint32_t>( deserialized_idata["energy_consumption"] ) );

    auto tokens_to_mint = std::get<int64_t>(deserialized_idata["mint"]);
    auto category = std::get<std::string>(deserialized_idata["category"]);

    auto asset_to_mint = asset{tokens_to_mint, CATEGORY_TO_TOKEN[category]};

    auto last_usage = std::get<uint32_t>(deserialized_mdata["used"]);

    deserialized_mdata["used"] = ++last_usage;
    deserialized_mdata["last_usage"] = current_time;

    action(
        { _self, ACTIVE_PERMISSION }, TOKEN_CONTRACT_NAME, "issue"_n,
        std::tuple(asset_owner, asset_to_mint, memo)
    ).send();

    action(
       { _self, ACTIVE_PERMISSION }, atomicassets::ATOMICASSETS_ACCOUNT, "setassetdata"_n,
        std::tuple(_self, _self, asset_id, deserialized_mdata)
    ).send();
}

ACTION nft::unstakenft( const name& asset_owner, const uint64_t asset_id ) {
    require_auth( _self );

    cards_t assets( _self, _self.value );
    auto owner = assets.find( asset_id );

    check( owner != assets.end(), "Asset owner does not exist" );
    check( owner->asset_owner == asset_owner, "Asset owner does not have NFT" );

    auto idata = get_asset_idata( _self, asset_id );
    auto category = name(std::get<std::string>(idata["category"]));

    amounts_t amounts( _self, _self.value );
    auto asset_key = uint128_t{asset_owner.value} << 64 | category.value;
    auto idx = amounts.get_index<"byownercat"_n>();
    auto amounts_of_user_and_cat = idx.find( asset_key );
    
    auto amt = amounts.find( amounts_of_user_and_cat->id );

    check( amt->nft_amount > 0, "No nft on account" );

    amounts.modify( amt, _self, [&]( auto& row ) {
        row.nft_amount--;
    });

    std::vector<uint64_t> assets_to_transfer{};
    assets_to_transfer.push_back(asset_id);

    action(
        { _self, ACTIVE_PERMISSION }, atomicassets::ATOMICASSETS_ACCOUNT, "transfer"_n,
        std::tuple( _self, owner->asset_owner, assets_to_transfer, nullptr)
    ).send();

    assets.erase(owner);
}

ACTION nft::consumefood( const name& account, const asset& food_to_consume  ) {
    require_auth( _self );
    check( food_to_consume.amount > 0, "Amount of food must be positive" );

    accounts_t accounts( _self, _self.value );
    auto acct = accounts.find( account.value );

    check( acct != accounts.end(), "No account" );

    auto quantities = acct->quantities;

    auto food_position = 0;
    for (auto quantity = quantities.begin(); quantity != quantities.end(); ++quantity) {
        if ( quantity->symbol == FOOD_SYMBOL ) {
            check( quantity->amount >= food_to_consume.amount, "Not enough food" );

            accounts.modify( acct, _self, [&]( auto& row ) {
                auto energy_amount = food_to_consume.amount * FOOD_TO_ENERGY_COEFFICIENT / ONE_TOKEN_IN_ATOMIC;
                check( row.energy + energy_amount <= MAX_ENERGY, "Energy limit exceeded" );

                row.energy += energy_amount;
                
                auto new_quantity = quantity->amount - food_to_consume.amount;
                row.quantities[food_position].set_amount(new_quantity);
            });

            break;
        }
        food_position++;
    }
}

ACTION nft::addasset( const name& asset_code, const name& collection_name, const name& schema_name,
                      const int32_t template_id, const uint32_t craft_energy_consumption, const uint32_t repair_energy_consumption,
                      const std::vector<asset>& craft_resources, const std::vector<asset>& repair_resources,
                      const std::string& description, const std::string& comment ) {
    require_auth( _self );

    tools_t assets( _self, _self.value );
    
    auto asset_with_name = assets.find( asset_code.value );

    if ( asset_with_name == assets.end() ) {
        assets.emplace( _self, [&]( auto& row ) {
            row.asset_code = asset_code;
            row.collection_name = collection_name;
            row.schema_name = schema_name;
            row.template_id = template_id;
            row.craft_energy_consumption = craft_energy_consumption;
            row.repair_energy_consumption = repair_energy_consumption;
            row.craft_resources = craft_resources;
            row.repair_resources = repair_resources;
            row.description = description;
            row.comment = comment;
        });
    } else {
        assets.modify( asset_with_name, _self, [&]( auto& row ) {
            row.collection_name = collection_name;
            row.schema_name = schema_name;
            row.template_id = template_id;
            row.craft_energy_consumption = craft_energy_consumption;
            row.repair_energy_consumption = repair_energy_consumption;
            row.craft_resources = craft_resources;
            row.repair_resources = repair_resources;
            row.description = description;
            row.comment = comment;
        });
    }
}

ACTION nft::remasset( const name& asset_name ) {
    require_auth( _self );

    tools_t assets( _self, _self.value );
    auto asset_with_name = assets.find( asset_name.value );

    check( asset_with_name != assets.end(), "No such asset" );

    assets.erase(asset_with_name);
}

void nft::ontransferft(const name& from, const name& to, const std::vector<asset>& quantities, const std::string& memo) {
    if ( to.value != _self.value ) {
        return;
    }

    action(
        { to, ACTIVE_PERMISSION }, TOKEN_CONTRACT_NAME, "burntransfer"_n,
        std::tuple( to, COMMISSION_ACCOUNT, quantities, COMMISSION, memo )
    ).send();

    accounts_t accounts( _self, _self.value );

    auto account = accounts.find( from.value );
    if (account == accounts.end()) {
        accounts.emplace( _self, [&]( auto& row ) {
            row.owner = from;
            row.energy = 0;
            row.quantities = std::vector<asset>();
        });
    }
    add_balances( from, quantities );
}

void nft::ontransfernft(const name& from, const name& to, const std::vector<uint64_t>& asset_ids, const std::string& memo) {
    if ( to.value != _self.value ) {
        return;
    }

    cards_t assets( _self, _self.value );
    atomicassets::assets_t nfts( atomicassets::ATOMICASSETS_ACCOUNT, to.value );

    amounts_t amounts( _self, _self.value );

    for (uint64_t i = 0; i < asset_ids.size(); i++) {
        auto current_asset_id = asset_ids[i];

        auto idata = get_asset_idata( to, current_asset_id );
        auto category = name(std::get<std::string>(idata["category"]));

        auto asset_key = uint128_t{from.value} << 64 | category.value;

        auto idx = amounts.get_index<"byownercat"_n>();
        auto amounts_of_user_and_cat = idx.find( asset_key );

        if (amounts_of_user_and_cat == idx.end()) {
            amounts.emplace( to, [&]( auto& row ) {
                row.id = amounts.available_primary_key();
                row.asset_owner = from;
                row.category = category;
                row.nft_amount = 1ULL;
            });
        } else {
            auto amt = amounts.find( amounts_of_user_and_cat->id );

            check( amt->nft_amount + 1 <= MAX_NFTS_PER_CATEGORY, "Too many NFTs of one category on one account" );

            amounts.modify( amt, to, [&]( auto& row ) {
                row.nft_amount++;
            });
        }

        assets.emplace( to, [&]( auto& row ) {
            row.asset_owner = from;
            row.category = category;
            row.asset_id = current_asset_id;
        });
    }
}

inline void nft::check_if_broken(ATTRIBUTE_MAP& idata, ATTRIBUTE_MAP& mdata) const {
    auto times_used = std::get<uint32_t>(mdata["used"]);
    auto max_usages = std::get<uint32_t>(idata["max_usages"]);

    check(times_used <= max_usages, "Broken");
}

inline void nft::check_if_not_broken(ATTRIBUTE_MAP& idata, ATTRIBUTE_MAP& mdata) const {
    auto times_used = std::get<uint32_t>(mdata["used"]);
    auto max_usages = std::get<uint32_t>(idata["max_usages"]);

    check(times_used >= max_usages, "Not broken");
}

inline void nft::check_if_on_cooldown(ATTRIBUTE_MAP& idata, ATTRIBUTE_MAP& mdata, const uint32_t current_time) const {
    auto last_usage = std::get<uint32_t>(mdata["last_usage"]);
    auto cooldown = std::get<uint32_t>(idata["cd"]);

    check(last_usage + cooldown < current_time, "On cooldown");
}

inline std::tuple<atomicdata::ATTRIBUTE_MAP, atomicdata::ATTRIBUTE_MAP> nft::get_asset_data(const name& asset_owner, const uint64_t& asset_id) {
    auto assets = atomicassets::get_assets(asset_owner);
    auto nft_itr = assets.find(asset_id);

    check(nft_itr != assets.end(), "User does not have this asset");

    auto schemas = atomicassets::get_schemas(nft_itr->collection_name);
    auto schema_itr = schemas.find((nft_itr->schema_name).value);

    check(schema_itr != schemas.end(), "No schema");

    auto template_id = nft_itr->template_id;
    auto templates = atomicassets::get_templates(nft_itr->collection_name);
    auto template_itr = templates.find(template_id);

    check(template_itr != templates.end(), "No template");

    auto deserialized_idata = atomicdata::deserialize(template_itr->immutable_serialized_data, schema_itr->format);
    check(deserialized_idata.size() > 0, "Empty idata");

    auto deserialized_mdata = atomicdata::deserialize(nft_itr->mutable_serialized_data, schema_itr->format);
    check(deserialized_mdata.size() > 0, "Empty mdata");

    return std::tuple(deserialized_idata, deserialized_mdata);
}

inline atomicdata::ATTRIBUTE_MAP nft::get_asset_idata(const name& asset_owner, const uint64_t& asset_id) {
    auto assets = atomicassets::get_assets(asset_owner);
    auto nft_itr = assets.find(asset_id);

    check(nft_itr != assets.end(), "User does not have this asset");

    auto schemas = atomicassets::get_schemas(nft_itr->collection_name);
    auto schema_itr = schemas.find((nft_itr->schema_name).value);

    check(schema_itr != schemas.end(), "No schema");

    auto template_id = nft_itr->template_id;
    auto templates = atomicassets::get_templates(nft_itr->collection_name);
    auto template_itr = templates.find(template_id);

    check(template_itr != templates.end(), "No template");

    auto deserialized_idata = atomicdata::deserialize(template_itr->immutable_serialized_data, schema_itr->format);
    check(deserialized_idata.size() > 0, "Empty idata");

    return deserialized_idata;
}

inline atomicdata::ATTRIBUTE_MAP nft::get_asset_mdata(const name& asset_owner, const uint64_t& asset_id) {
    auto assets = atomicassets::get_assets(asset_owner);
    auto nft_itr = assets.find(asset_id);

    check(nft_itr != assets.end(), "User does not have this asset");

    auto schemas = atomicassets::get_schemas(nft_itr->collection_name);
    auto schema_itr = schemas.find((nft_itr->schema_name).value);

    check(schema_itr != schemas.end(), "No schema");

    auto deserialized_mdata = atomicdata::deserialize(nft_itr->mutable_serialized_data, schema_itr->format);
    check(deserialized_mdata.size() > 0, "Empty mdata");

    return deserialized_mdata;
}

/**
 * @brief Owner should present in accounts table.
 * 
 * @param owner 
 * @param assets 
 */
void nft::add_balances( const name& owner, const std::vector<asset>& quantities ) {
    accounts_t accounts( _self, _self.value );
    auto acct = accounts.find( owner.value );

    check( acct != accounts.end(), "No account" );

    accounts.modify( acct, _self, [&]( auto& row ) {
        std::vector<asset>& account_quantities = row.quantities;
        for (uint32_t i = 0; i < quantities.size(); ++i) {
            bool found = false;
            auto& qty = quantities[i];

            for (uint32_t j = 0; j < account_quantities.size(); ++j) {
                if (qty.symbol.raw() == account_quantities[j].symbol.raw()) {
                    account_quantities[j] += qty;
                    found = true;
                }
            }

            if (!found) {
                account_quantities.push_back(quantities[i]);
            }
        }
    });
}

void nft::sub_balances( const name& owner, const std::vector<asset>& quantities ) {
    accounts_t accounts( _self, _self.value );
    auto acct = accounts.find( owner.value );

    check( acct != accounts.end(), "No account" );

    accounts.modify( acct, _self, [&]( auto& row ) {
        std::vector<asset>& account_quantities = row.quantities;
        for (uint32_t i = 0; i < quantities.size(); ++i) {
            bool found = false;
            auto& qty = quantities[i];

            for (uint32_t j = 0; j < account_quantities.size(); ++j) {
                if (qty.symbol == account_quantities[j].symbol) {
                    check( acct->quantities[j] >= qty, "Insufficient funds" );

                    account_quantities[j] -= qty;
                    found = true;
                }
            }

            check( found, "Insufficient funds (0)" );
        }
    });
}

void nft::consume_energy( const name& account, const uint32_t energy_consumption ) {
    accounts_t accounts( _self, _self.value );
    auto consumer = accounts.find( account.value );

    check( consumer != accounts.end(), "Asset owner must be registered" );
    check( consumer->energy >= energy_consumption, "Insufficient energy" );

    accounts.modify( consumer, _self, [&]( auto& row ) {
        row.energy -= energy_consumption;
    });
}
