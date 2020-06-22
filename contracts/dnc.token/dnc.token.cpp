/**
 *  @file
 *  @copyright defined in dnc/LICENSE.txt
 */

#include <dncio.token/dncio.token.hpp>

namespace dncio {

void token::create( account_name issuer,
                    asset        maximum_supply )
{
    auto sym = maximum_supply.symbol;
    dncio_assert( sym.is_valid(), "invalid symbol name" );
    dncio_assert( maximum_supply.is_valid(), "invalid supply");
    dncio_assert( maximum_supply.amount > 0, "max-supply must be positive");
    dncio::symbol_type dnc = S(4,dnc);
    dncio_assert( sym.name() != dnc.name(), "not create dnc");

    stats statstable( _self, sym.name() );
    auto existing = statstable.find( sym.name() );
    dncio_assert( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( _self, [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
    });
}


void token::issue( account_name to, asset quantity, string memo )
{
    auto sym = quantity.symbol;
    dncio_assert( sym.is_valid(), "invalid symbol name" );
    dncio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto sym_name = sym.name();
    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    dncio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    require_auth( st.issuer );
    dncio_assert( quantity.is_valid(), "invalid quantity" );
    dncio_assert( quantity.amount > 0, "must issue positive quantity" );

    dncio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    dncio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply += quantity;
    });

    add_balance( st.issuer, quantity, st.issuer );

    if( to != st.issuer ) {
       SEND_INLINE_ACTION( *this, transfer, {st.issuer,N(active)}, {st.issuer, to, quantity, memo} );
    }
}

void token::transfer( account_name from,
                      account_name to,
                      asset        quantity,
                      string       memo )
{
    dncio_assert( from != to, "cannot transfer to self" );
    require_auth( from );
    dncio_assert( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.name();
    stats statstable( _self, sym );
    const auto& st = statstable.get( sym );

    require_recipient( from );
    require_recipient( to );

    dncio_assert( quantity.is_valid(), "invalid quantity" );
    dncio_assert( quantity.amount > 0, "must transfer positive quantity" );
    dncio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    dncio_assert( memo.size() <= 256, "memo has more than 256 bytes" );


    sub_balance( from, quantity );
    add_balance( to, quantity, from );
}

void token::sub_balance( account_name owner, asset value ) {
   accounts from_acnts( _self, owner );

   const auto& from = from_acnts.get( value.symbol.name(), "no balance object found" );
   dncio_assert( from.balance.amount >= value.amount, "overdrawn balance" );


   if( from.balance.amount == value.amount ) {
      from_acnts.erase( from );
   } else {
      from_acnts.modify( from, owner, [&]( auto& a ) {
          a.balance -= value;
      });
   }
}

void token::add_balance( account_name owner, asset value, account_name ram_payer )
{
   accounts to_acnts( _self, owner );
   auto to = to_acnts.find( value.symbol.name() );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });
   } else {
      to_acnts.modify( to, 0, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

} /// namespace dncio

dncIO_ABI( dncio::token, (create)(issue)(transfer) )
