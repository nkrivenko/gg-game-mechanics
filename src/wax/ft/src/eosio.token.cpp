#include <eosio.token/eosio.token.hpp>

namespace eosio {

void token::create( const name&   issuer,
                    const asset&  maximum_supply )
{
    require_auth( get_self() );

    auto sym = maximum_supply.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( maximum_supply.is_valid(), "invalid supply");
    check( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( get_self(), [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
    });
}

void token::issue( const name& to, const asset& quantity, const string& memo )
{
    check_minter();
    auto sym = quantity.symbol;
    
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "No token with such symbol" );
    const auto& st = *existing;

    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
        s.supply += quantity;
    });

    add_balance( to, quantity, st.issuer );
}

void token::retire( const asset& quantity, const string& memo )
{
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must retire positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply -= quantity;
    });

    sub_balance( st.issuer, quantity );
}

void token::burn( const name& from, const asset& quantity, const string& memo ) {
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    const auto& st = statstable.get( sym.code().raw(), "token with symbol does not exist" );

    require_auth( from );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must burn positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    statstable.modify( st, from, [&]( auto& s ) {
        s.supply -= quantity;
    });

    sub_balance( from, quantity );
}

void token::transfer( const name&    from,
                      const name&    to,
                      const asset&   quantity,
                      const string&  memo )
{
    check( from != to, "cannot transfer to self" );
    require_auth( from );
    check( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.code();
    stats statstable( get_self(), sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );

    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must transfer positive quantity" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    auto payer = has_auth( to ) ? to : from;

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
}

void token::sub_balance( const name& owner, const asset& value ) {
   accounts from_acnts( get_self(), owner.value );

   const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
   check( from.balance.amount >= value.amount, "overdrawn balance" );

   from_acnts.modify( from, owner, [&]( auto& a ) {
         a.balance -= value;
      });
}

void token::add_balance( const name& owner, const asset& value, const name& ram_payer )
{
   accounts to_acnts( get_self(), owner.value );
   auto to = to_acnts.find( value.symbol.code().raw() );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });
   } else {
      to_acnts.modify( to, same_payer, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

void token::open( const name& owner, const symbol& symbol, const name& ram_payer )
{
   require_auth( ram_payer );

   check( is_account( owner ), "owner account does not exist" );

   auto sym_code_raw = symbol.code().raw();
   stats statstable( get_self(), sym_code_raw );
   const auto& st = statstable.get( sym_code_raw, "symbol does not exist" );
   check( st.supply.symbol == symbol, "symbol precision mismatch" );

   accounts acnts( get_self(), owner.value );
   auto it = acnts.find( sym_code_raw );
   if( it == acnts.end() ) {
      acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = asset{0, symbol};
      });
   }
}

void token::close( const name& owner, const symbol& symbol )
{
   require_auth( owner );
   accounts acnts( get_self(), owner.value );
   auto it = acnts.find( symbol.code().raw() );
   check( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
   check( it->balance.amount == 0, "Cannot close because the balance is not zero." );
   acnts.erase( it );
}

void token::transfers( const name& from, const name& to, const std::vector<asset>& quantities, const string& memo ) {
    check( from != to, "cannot transfer to self" );
    check( is_account( to ), "to account does not exist");
    check( memo.size() <= 256, "memo has more than 256 bytes" );
    check( quantities.size() > 0, "quantities must not be empty" );

    require_auth( from );

    auto payer = has_auth( to ) ? to : from;

    for ( auto it = quantities.cbegin(); it != quantities.cend(); it++ ) {
        const auto& quantity = *it;

        auto sym = quantity.symbol.code();
        stats statstable( get_self(), sym.raw() );
        const auto& st = statstable.get( sym.raw() );

        require_recipient( from );
        require_recipient( to );

        check( quantity.is_valid(), "invalid quantity" );
        check( quantity.amount > 0, "must transfer positive quantity" );
        check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

        sub_balance( from, quantity );
        add_balance( to, quantity, payer );
    }
}

void token::burntransfer( const name& from, const name& to, const std::vector<asset>& quantities, const int16_t transfer_percent, const string& memo ) {
    require_auth( from );
    check(transfer_percent <= 100 && transfer_percent > 0, "Transfer percent must be positive number less than or equal to 100");

    std::vector<asset> transfer_amounts{};

    for (auto it = quantities.begin(); it != quantities.end(); it++) {
        auto asset_to_bt = *it;
        auto amount = asset_to_bt.amount;

        auto tokens_to_transfer = eosio::asset((int64_t) (it->amount * transfer_percent / 100ULL), asset_to_bt.symbol);
        auto tokens_to_burn = eosio::asset{it->amount - tokens_to_transfer.amount, asset_to_bt.symbol};

        burn(from, tokens_to_burn, memo);

        transfer_amounts.push_back(tokens_to_transfer);
    }

    transfers(from, to, transfer_amounts, memo);
}

void token::addminter( const name& new_minter ) {
    check( new_minter != _self, "Cannot add self" );

    require_auth( _self );
    minters minters( _self, _self.value );

    auto iter = minters.find( new_minter.value );
    check( iter == minters.end(), "Already a minter" );

    minters.emplace(get_self(), [&](auto& row) {
        row.minter = new_minter;
    });
}

void token::remminter( const name& old_minter ) {
    check( old_minter != _self, "Cannot add self" );

    require_auth( get_self() );

    minters minters( get_self(), _self.value );

    const auto iter = minters.find( old_minter.value );
    check( iter != minters.end(), "Not a minter" );

    minters.erase( iter );
}

void token::check_minter() const {
  	bool auth = false;

  	if (has_auth(_self)) {
  		auth = true;
	} else {
	    minters minters(_self, _self.value);

        for( auto itr = minters.cbegin(); itr != minters.cend(); itr++ ) {
            if (eosio::has_auth(itr->minter)) {
                auth = true;
                break;
            }
        }
    }

    check(auth, "Unauthorized");
}

} /// namespace eosio
