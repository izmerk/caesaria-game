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

#ifndef _CAESARIA_TILESARRAY_INCLUDE_H_
#define _CAESARIA_TILESARRAY_INCLUDE_H_

#include "tile.hpp"
#include "core/tilepos_array.hpp"

#include <vector>
#include <cstring>

namespace gfx
{

class TilesArray : public std::vector<Tile*>
{
public:
  struct Corners
  {
    TilePos leftup;
    TilePos leftdown;
    TilePos rightdown;
    TilePos rightup;
  };

  bool contain( const TilePos& tilePos ) const;

  TilesArray() {}

  TilesArray( const TilesArray& a );

  Corners corners() const;

  TilePos leftUpCorner() const;

  TilePos rightDownCorner() const;

  TilesArray& operator=(const TilesArray& a);

  TilesArray& append( const TilesArray& a );
  TilesArray& append( Tile* a );

  TilesArray walkables( bool alllands=false ) const;
  TilesArray select( Tile::Type flag ) const;

  TilesArray terrains() const;

  TilesArray waters() const;

  TilesArray& remove(const TilePos &pos );
  TilePosArray locations() const;

  OverlayList overlays() const;

  void pop_front();

  Tile* random() const;
};

}//end namespace

#endif //_CAESARIA_TILESARRAY_INCLUDE_H_
