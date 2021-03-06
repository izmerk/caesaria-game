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

#include "city/city.hpp"
#include "mercury.hpp"
#include "objects/warehouse.hpp"
#include "objects/granary.hpp"
#include "events/showinfobox.hpp"
#include "game/gamedate.hpp"
#include "core/gettext.hpp"
#include "good/store.hpp"
#include "core/format.hpp"
#include "objects/extension.hpp"
#include "objects/factory.hpp"
#include "city/statistic.hpp"
#include "core/utils.hpp"

using namespace gfx;

namespace religion
{

namespace rome
{

void Mercury::updateRelation(float income, PlayerCityPtr city)
{
  RomeDivinity::updateRelation( income, city );
}

object::Type Mercury::templeType(Divinity::TempleSize size) const
{
    return size == bigTemple
                    ? BIG_TEMPLE_TYPE(mercury)
                    : SML_TEMPLE_TYPE(mercury);
}

Mercury::Mercury()
  : RomeDivinity( RomeDivinity::Mercury )
{

}

template<class T>
void __filchGoods( const std::string& title, PlayerCityPtr city, bool showMessage )
{
  if( showMessage )
  {
    std::string txt = fmt::format( "##{0}_of_mercury_title##", title );
    std::string descr = fmt::format( "##{0}_of_mercury_description##", title );

    events::dispatch<events::ShowInfobox>( _(txt),
                                           _(descr),
                                           true,
                                           "god_mercury");
  }

  SmartList<T> buildings = city->statistic().objects.find<T>();

  for( auto building : buildings )
  {
    good::Store& store = building->store();
    for( auto& gtype : good::all() )
    {
      int goodQty = math::random( (store.qty( gtype ) + 99) / 100 ) * 100;
      if( goodQty > 0 )
      {
        good::Stock rmStock( gtype, goodQty );
        store.retrieve( rmStock, goodQty );
      }
    }
  }
}

void Mercury::_doWrath(PlayerCityPtr city)
{
  __filchGoods<Warehouse>( "wrath", city, true );
  __filchGoods<Granary>( "smallcurse", city, false );
}

void Mercury::_doSmallCurse(PlayerCityPtr city)
{
  events::dispatch<events::ShowInfobox>( _("##smallcurse_of_mercury_title##"),
                                         _("##smallcurse_of_mercury_description##") );

  FactoryList factories = city->overlays().select<Factory>();

  for( auto factory : factories )
  {
    FactoryProgressUpdater::assignTo( factory, -5, 4 * 12 );
  }
}

void Mercury::_doBlessing(PlayerCityPtr city)
{
  auto warehouses = city->overlays().select<Warehouse>();

  for( auto wh : warehouses )
  {
    WarehouseBuff::assignTo( wh, Warehouse::sellGoodsBuff, 0.2, 4 * 12 );
  }
}

}//end namespace rome

}//end namespace religion
