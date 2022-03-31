#include <string>
#include <array>
#include <algorithm>

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>

#include "atomicassets.hpp"
#include "hydra.hpp"
#include "serialization/data.hpp"

#define TOKEN_CONTRACT_NAME "ggttokenwat1"_n
#define TOKEN_CONTRACT_NAME_TRANSFERS_ACTION "ggttokenwat1::transfers"
#define ATOMICASSETS_TRANSFER_ACTION "atomicassets::transfer"

using namespace eosio;

static const atomicdata::ATTRIBUTE_MAP REPAIRED_ASSET_DATA = {
    {"used", 0u},
    {"last_usage", 0u}
};

static constexpr uint8_t SYMBOL_PRECISION = 4;
static constexpr int16_t ONE_TOKEN_IN_ATOMIC = 10000;
static constexpr uint8_t FOOD_TO_ENERGY_COEFFICIENT = 5;
static constexpr uint64_t MAX_NFTS_PER_CATEGORY = 2;
static constexpr uint32_t MAX_ENERGY = 500000;
static constexpr int16_t COMMISSION = 15;
static constexpr name COMMISSION_ACCOUNT = "nkrivenko351"_n;
static constexpr name ACTIVE_PERMISSION = "active"_n;

static constexpr symbol FOOD_SYMBOL = symbol("GF", SYMBOL_PRECISION);
static constexpr symbol WATER_SYMBOL = symbol("GW", SYMBOL_PRECISION);
static constexpr symbol METAL_SYMBOL = symbol("GM", SYMBOL_PRECISION);

static const std::string WATER_CATEGORY = "water";
static const std::string FOOD_CATEGORY = "food";
static const std::string METAL_CATEGORY = "metal";

static std::map<std::string, symbol> CATEGORY_TO_TOKEN = {
    {WATER_CATEGORY, WATER_SYMBOL},
    {FOOD_CATEGORY, FOOD_SYMBOL},
    {METAL_CATEGORY, METAL_SYMBOL}
};

CONTRACT nft : public contract {
    private:
        inline void check_if_broken(ATTRIBUTE_MAP& idata, ATTRIBUTE_MAP& mdata) const;
        inline void check_if_on_cooldown(ATTRIBUTE_MAP& idata, ATTRIBUTE_MAP& mdata, const uint32_t current_time) const;
        inline void check_if_not_broken(ATTRIBUTE_MAP& idata, ATTRIBUTE_MAP& mdata) const;

        inline std::tuple<atomicdata::ATTRIBUTE_MAP, atomicdata::ATTRIBUTE_MAP> get_asset_data(const name& asset_owner, const uint64_t& asset_id);
        inline atomicdata::ATTRIBUTE_MAP get_asset_mdata(const name& asset_owner, const uint64_t& asset_id);
        inline atomicdata::ATTRIBUTE_MAP get_asset_idata(const name& asset_owner, const uint64_t& asset_id);

        void add_balances( const name& owner, const std::vector<asset>& quantities );
        void sub_balances( const name& owner, const std::vector<asset>& quantities );

        void consume_energy( const name& account, const uint32_t energy_consumption );
    public:
        using contract::contract;

        /**
         * @brief Crafts new NFT, ownership goes to the `owner`
         * 
         * Resource tokens needed for the repair will be burned.
         * 
         * @require_auth self
         * @param asset_code Name of NFT
         * @param owner Owner of crafted NFT
         */
        ACTION craft( const name& asset_code, const name& asset_owner );

        /**
         * @brief Resets the count of usage and last_used time.
         * 
         * Cannot be called on non-broken NFTs or NFTs on cooldown.
         * Resource tokens needed for the repair will be burned.
         * 
         * @require_auth self
         * @param asset_owner NFT owner
         * @param asset_id AtomicAssets NFT ID
         */
        ACTION repair( const name& asset_owner, const uint64_t asset_id );

        /**
         * @brief Mines the resource tokens based on type of the NFT passed.
         * 
         * @param asset_owner 
         * @param asset_id 
         * @param memo 
         * @return ACTION 
         */
        ACTION mine( const name& asset_owner, const uint64_t asset_id, const std::string& memo );

        /**
         * @brief Transfers the NFT back to owner.
         * 
         * @param asset_owner previous NFT owner
         * @param asset_id AtomicAssets asset ID
         */
        ACTION unstakenft( const name& asset_owner, const uint64_t asset_id );

        /**
         * @brief Consumes food, burning food tokens and gaining energy.
         * 
         * @param account Account to consume food.
         * @param food_to_consume Amount of food tokens to consume
         */
        ACTION consumefood( const name& account, const asset& food_to_consume  );

        /**
         * @brief Add NFT characteristics to craft and repair.
         * 
         * @require_auth self
         * @param asset_code NFT name
         * @param collection_name AtomicAssets collection name
         * @param schema_name AtomicAssets schema name
         * @param template_id AtomicAssets template id
         * @param quantities Cost of craft
         */
        ACTION addasset( const name& asset_code, const name& collection_name, const name& schema_name,
                         const int32_t template_id, const uint32_t craft_energy_consumption, const uint32_t repair_energy_consumption,
                         const std::vector<asset>& craft_resources, const std::vector<asset>& repair_resources,
                         const std::string& description, const std::string& comment );

        /**
         * @brief Remove NFT characteristics.
         *
         * @require_auth self
         * @param asset_code NFT name
         * @return ACTION 
         */
        ACTION remasset( const name& asset_code );

// #ifdef HYDRA_ENV
        HYDRA_FIXTURE_ACTION(
            ((accounts)(accounts_s)(accounts_t))
            ((amounts)(amounts_s)(amounts_t))
            ((assets)(cards_s)(cards_t))
            ((nfts)(tools_s)(tools_t))
        )
// #endif

        struct [[eosio::table]] accounts_s {
            name owner;
            uint32_t energy;
            std::vector<asset> quantities;

            uint64_t primary_key() const { return owner.value; };
        };

        struct [[eosio::table]] cards_s {
            name asset_owner;
            name category;
            uint64_t asset_id;

            uint64_t primary_key() const { return asset_id; }
            uint64_t byowner() const { return asset_owner.value; }
            uint128_t byownercat() const { return uint128_t{asset_owner.value} << 64 | category.value; }
        };

        struct [[eosio::table]] amounts_s {
            uint64_t id;
            name asset_owner;
            name category;
            uint64_t nft_amount;

            uint64_t primary_key() const { return id; }
            uint128_t byownercat() const { return uint128_t{asset_owner.value} << 64 | category.value; }
        };

        struct [[eosio::table]] tools_s {
            name asset_code;
            name collection_name;
            name schema_name;
            int32_t template_id;

            uint32_t craft_energy_consumption;
            uint32_t repair_energy_consumption;

            std::vector<asset> craft_resources;
            std::vector<asset> repair_resources;

            std::string description;
            std::string comment;

            uint64_t primary_key() const { return asset_code.value; }
        };

        typedef multi_index<name("accounts"), accounts_s> accounts_t;
        typedef multi_index<
            name("amounts"), amounts_s,
            indexed_by<"byownercat"_n, const_mem_fun<amounts_s, uint128_t, &amounts_s::byownercat>>
        > amounts_t;
        typedef multi_index<
            name("assets"), cards_s,
            indexed_by<"byowner"_n, const_mem_fun<cards_s, uint64_t, &cards_s::byowner>>,
            indexed_by<"byownercat"_n, const_mem_fun<cards_s, uint128_t, &cards_s::byownercat>>
        > cards_t;
        typedef multi_index<name("nfts"), tools_s> tools_t;

        [[eosio::on_notify(TOKEN_CONTRACT_NAME_TRANSFERS_ACTION)]]
        void ontransferft(const name& from, const name& to, const std::vector<asset>& quantity, const std::string& memo);

        [[eosio::on_notify(ATOMICASSETS_TRANSFER_ACTION)]]
        void ontransfernft(const name& from, const name& to, const std::vector<uint64_t>& asset_ids, const std::string& memo);

        using craft_action = action_wrapper<"craft"_n, &nft::craft>;
        using repair_action = action_wrapper<"repair"_n, &nft::repair>;
        using mine_action = action_wrapper<"mine"_n, &nft::mine>;
        using unstake_nft_action = action_wrapper<"unstakenft"_n, &nft::unstakenft>;
        using consume_food_action = action_wrapper<"consumefood"_n, &nft::consumefood>;
        using add_asset_action = action_wrapper<"addasset"_n, &nft::addasset>;
        using remove_asset_action = action_wrapper<"remasset"_n, &nft::remasset>;
};
