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

#include "workerhunter.hpp"
#include "objects/house.hpp"
#include "core/predefinitions.hpp"
#include "core/safetycast.hpp"
#include "core/position.hpp"
#include "servicewalker_helper.hpp"
#include "city/city.hpp"
#include "game/enums.hpp"
#include "game/resourcegroup.hpp"
#include "pathway/path_finding.hpp"
#include "constants.hpp"
#include "corpse.hpp"
#include "core/foreach.hpp"
#include "helper.hpp"

using namespace constants;

Recruter::Recruter(PlayerCityPtr city )
 : ServiceWalker( city, Service::recruter )
{    
  _workersNeeded = 0;
  _setType( walker::recruter );
}

void Recruter::hireWorkers( const int workers )
{
  WorkingBuildingPtr wbase = ptr_cast<WorkingBuilding>( getBase() );
  if( wbase.isValid() ) 
  {
    _workersNeeded = math::clamp( _workersNeeded - workers, 0, 0xff );
    wbase->addWorkers( workers );
  }
}

int Recruter::getWorkersNeeded() const { return _workersNeeded; }

void Recruter::_centerTile()
{
  Walker::_centerTile();

  if( _workersNeeded )
  {
    ServiceWalkerHelper hlp( *this );
    std::set<HousePtr> houses = hlp.getReachedBuildings<House>( pos() );

    foreach( it, houses ) { (*it)->applyService( ServiceWalkerPtr( this ) ); }
  }
  else
  {    
    if( !_pathwayRef().isReverse() ) //return2Base();
    {
      _pathwayRef().toggleDirection();
    }
  }
}

RecruterPtr Recruter::create(PlayerCityPtr city )
{ 
  RecruterPtr ret( new Recruter( city ) );
  ret->initialize( WalkerHelper::getOptions( ret->type() ) );

  ret->drop();
  return ret;
}

void Recruter::send2City( WorkingBuildingPtr building, const int workersNeeded )
{
  _workersNeeded = workersNeeded;
  ServiceWalker::send2City( building.object() );
}

void Recruter::die()
{
  ServiceWalker::die();

  Corpse::create( _city(), pos(), ResourceGroup::citizen1, 97, 104 );
}
