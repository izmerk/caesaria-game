// This file is part of CaesarIA.
//
// CaesarIA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CaesarIA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CaesarIA.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012-2014 Dalerank, dalerankn8@gmail.com

#include "rome.hpp"
#include <GameWorld>
#include <GameGood>
#include <GameEvents>
#include <GameApp>
#include <GameObjects>
#include <GameCore>
#include <GameCity>

using namespace events;

namespace world {

namespace {
const int maxSoldiers = 200;
const Point defaultLocation = Point( 870, 545 );
const int defaultPopulation = 45000;
}

const char* Rome::defaultName = "Rome";

class Rome::Impl
{
public:
  econ::Treasury funds;
  good::Storage gstore;
  DateTime lastAttack;
  city::States states;
  int strength;
};

Rome::Rome(EmpirePtr empire)
   : City( empire ), _d( new Impl )
{
  setLocation( defaultLocation );
  _d->strength = maxSoldiers;
  _d->states.age = 500;
  _setNation( nation::roman );
  _d->states.population = defaultPopulation;
}

CityPtr Rome::create(EmpirePtr empire)
{
  CityPtr ret( new Rome(empire) );
  ret->drop();

  return ret;
}

unsigned int Rome::tradeType() const { return 0; }
econ::Treasury& Rome::treasury() { return _d->funds; }

std::string Rome::name() const { return Rome::defaultName; }
bool Rome::isPaysTaxes() const { return true; }
const city::States& Rome::states() const { return _d->states; }

std::string Rome::about(Object::AboutType type)
{
  if( type == aboutEmtype )
    return "world_romancapital";

  return "##empiremap_capital##";
}

void Rome::timeStep(const unsigned int time)
{
  City::timeStep( time );
}

PlayerPtr Rome::mayor() const { return 0; }
bool Rome::haveOverduePayment() const { return false; }
const good::Store& Rome::buys() const{ return _d->gstore; }
void Rome::delayTrade(unsigned int) {}
void Rome::empirePricesChanged(good::Product, const PriceInfo&){}
const good::Store& Rome::sells() const{ return _d->gstore; }

void Rome::addObject(ObjectPtr obj)
{
  if( obj.is<GoodCaravan>() )
  {
    auto goodCaravan = obj.as<GoodCaravan>();

    good::Product gtype = good::none;
    for( auto& i : good::all())
    {
      if( goodCaravan->store().qty( i ) > 0 )
      {
        gtype = i;
        break;
      }
    }

    events::dispatch<ShowInfobox>( _("##rome_gratitude_request_title##"),
                                   _("##rome_gratitude_request_text##"),
                                   gtype,
                                   false );
  }  
  else if( obj.is<Barbarian>() )
  {
    _d->lastAttack = game::Date::current();
  }
}

void Rome::load(const VariantMap& stream)
{
  City::load( stream );
}

DateTime Rome::lastAttack() const { return _d->lastAttack; }
int Rome::strength() const { return _d->strength; }

} // end namespace world
