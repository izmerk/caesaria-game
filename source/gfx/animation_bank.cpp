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
// Copyright 2012-2013 Gregoire Athanase, gathanase@gmail.com
// Copyright 2012-2013 Dalerank, dalerankn8@gmail.com


#include "gfx/animation_bank.hpp"
#include "core/position.hpp"
#include "game/resourcegroup.hpp"
#include "gfx/picture.hpp"
#include "core/logger.hpp"
#include "core/saveadapter.hpp"
#include "good/goodhelper.hpp"
#include "walker/helper.hpp"
#include "picture_info_bank.hpp"
#include "core/variant_map.hpp"

using namespace constants;

namespace gfx
{

namespace{
  static const Point frontCartOffsetSouth = Point( -33, 22 );
  static const Point frontCartOffsetWest  = Point( -31, 35 );
  static const Point frontCartOffsetNorth = Point(  -5, 37 );
  static const Point frontCartOffsetEast  = Point(  -5, 22 );
  static const Point frontCartOffsetSouthEast  = Point( -20, 20 );
  static const Point frontCartOffsetNorthWest  = Point( -20, 40 );
  static const Point frontCartOffsetNorthEast  = Point( -5, 30 );
  static const Point frontCartOffsetSouthWest  = Point( -5, 22 );

  static const Point backCartOffsetSouth = Point(  -5, 40 );
  static const Point backCartOffsetWest  = Point(  -5, 22 );
  static const Point backCartOffsetNorth = Point( -33, 22 );
  static const Point backCartOffsetEast  = Point( -31, 35 );
  static const Point backCartOffsetSouthEast  = Point( -20, 40 );
  static const Point backCartOffsetNorthWest  = Point( -20, 20 );
  static const Point backCartOffsetNorthEast  = Point( -30, 30 );
  static const Point backCartOffsetSouthWest  = Point( -0, 30 );

  static const int defaultStepInFrame = 8;
}

struct ActionAnimation
{
  int ownerType;
  AnimationBank::MovementAnimation actions;
};

class AnimationBank::Impl
{
public:
  typedef enum { stgSimple, stgObjects, stgCarts } LoadingStage;
  typedef std::map< int, Animation > SimpleAnimations;
  typedef std::map< int, ActionAnimation > DirectedAnimations;
  typedef std::map< int, VariantMap > AnimationConfigs;

  AnimationConfigs animConfigs;
  
  DirectedAnimations objects;
  DirectedAnimations carts;
  SimpleAnimations simpleAnimations;

  // fills the cart pictures
  // prefix: image prefix
  // start: index of the first frame
  void fixCartOffset( int type, bool back );
  void loadAnimation( DirectedAnimations& refMap, int who, const std::string& prefix,
                     const int start, const int size,
                     Walker::Action wa=Walker::acMove,
                     const int step = defaultStepInFrame, int delay=0);
  void loadAnimation(int who, const VariantMap& desc, LoadingStage stage);
  const AnimationBank::MovementAnimation& tryLoadAnimations(int wtype );

  void loadCarts( vfs::Path model );
};

void AnimationBank::Impl::loadCarts(vfs::Path model)
{
  //number of animations with goods + emmigrants + immigrants
  bool frontCart = false;

  VariantMap config = SaveAdapter::load( model );
  foreach( it, config )
  {
    good::Type gtype = good::Helper::getType( it->first );

    if( gtype != good::none )
    {
      VariantMap cartInfo = it->second.toMap();
      Variant smallInfo = cartInfo.get( "small" );
      if( smallInfo.isValid() ) loadAnimation( gtype, smallInfo.toMap(), stgCarts );

      Variant bigInfo = cartInfo.get( "big" );
      if( bigInfo.isValid() ) loadAnimation( gtype + animBigCart, bigInfo.toMap(), stgCarts );

      Variant megaInfo = cartInfo.get( "mega" );
      if( megaInfo.isValid() ) loadAnimation( gtype + animMegaCart, megaInfo.toMap(), stgCarts );
    }
  }

  VariantMap imPureCart = config.get( "none" ).toMap();
  loadAnimation( good::none, imPureCart, stgCarts );

  //carts[good::none]     = fillCart( ResourceGroup::carts, noneGoodsPicId, frontCart);
  /*carts[good::wheat]    = fillCart( ResourceGroup::carts, 9, frontCart);
  carts[good::vegetable]= fillCart( ResourceGroup::carts, 17, frontCart);
  carts[good::fruit]    = fillCart( ResourceGroup::carts, 25, frontCart);
  carts[good::olive]    = fillCart( ResourceGroup::carts, 33, frontCart);
  carts[good::grape]    = fillCart( ResourceGroup::carts, 41, frontCart);
  carts[good::meat]     = fillCart( ResourceGroup::carts, 49, frontCart);
  carts[good::wine]     = fillCart( ResourceGroup::carts, 57, frontCart);
  carts[good::oil]      = fillCart( ResourceGroup::carts, 65, frontCart);
  carts[good::iron]     = fillCart( ResourceGroup::carts, 73, frontCart);
  carts[good::timber]   = fillCart( ResourceGroup::carts, 81, frontCart);
  carts[good::clay]     = fillCart( ResourceGroup::carts, 89, frontCart);
  carts[good::marble]   = fillCart( ResourceGroup::carts, 97, frontCart);
  carts[good::weapon]   = fillCart( ResourceGroup::carts, 105, frontCart);
  carts[good::furniture]= fillCart( ResourceGroup::carts, 113, frontCart);
  carts[good::pottery]  = fillCart( ResourceGroup::carts, 121, frontCart);
  carts[good::fish]     = fillCart( ResourceGroup::carts, 697, frontCart);

  carts[animBigCart+good::none]     = fillCart( ResourceGroup::carts, noneGoodsPicId, frontCart);
  carts[animBigCart+good::wheat]    = fillCart( ResourceGroup::carts, 617, frontCart);
  carts[animBigCart+good::vegetable]= fillCart( ResourceGroup::carts, 625, frontCart);
  carts[animBigCart+good::fruit]    = fillCart( ResourceGroup::carts, 633, frontCart);
  carts[animBigCart+good::meat]     = fillCart( ResourceGroup::carts, 641, frontCart);
  carts[animBigCart+good::fish]     = fillCart( ResourceGroup::carts, 649, frontCart);

  carts[animBigCart+good::olive]    = fillCart( ResourceGroup::citizen5, 97, frontCart);
  carts[animBigCart+good::grape]    = fillCart( ResourceGroup::carts, 105, frontCart);
  carts[animBigCart+good::wine]     = fillCart( ResourceGroup::carts, 113, frontCart);
  carts[animBigCart+good::oil]      = fillCart( ResourceGroup::carts, 121, frontCart);
  carts[animBigCart+good::iron]     = fillCart( ResourceGroup::carts, 129, frontCart);
  carts[animBigCart+good::timber]   = fillCart( ResourceGroup::carts, 137, frontCart);
  carts[animBigCart+good::clay]     = fillCart( ResourceGroup::carts, 145, frontCart);
  carts[animBigCart+good::marble]   = fillCart( ResourceGroup::carts, 153, frontCart);
  carts[animBigCart+good::weapon]   = fillCart( ResourceGroup::carts, 161, frontCart);
  carts[animBigCart+good::furniture]= fillCart( ResourceGroup::carts, 169, frontCart);
  carts[animBigCart+good::pottery]  = fillCart( ResourceGroup::carts, 177, frontCart);

  carts[animMegaCart+good::none]     = fillCart( ResourceGroup::carts, noneGoodsPicId, frontCart);
  carts[animMegaCart+good::wheat]    = fillCart( ResourceGroup::carts, 547, frontCart);
  carts[animMegaCart+good::vegetable]= fillCart( ResourceGroup::carts, 665, frontCart);
  carts[animMegaCart+good::fruit]    = fillCart( ResourceGroup::carts, 673, frontCart);
  carts[animMegaCart+good::meat]     = fillCart( ResourceGroup::carts, 681, frontCart);
  carts[animMegaCart+good::fish]     = fillCart( ResourceGroup::carts, 689, frontCart);

  carts[animMegaCart+good::grape]    = fillCart( ResourceGroup::carts, 41, frontCart);
  carts[animMegaCart+good::olive]    = fillCart( ResourceGroup::carts, 33, frontCart);
  carts[animMegaCart+good::wine]     = fillCart( ResourceGroup::carts, 57, frontCart);
  carts[animMegaCart+good::oil]      = fillCart( ResourceGroup::carts, 65, frontCart);
  carts[animMegaCart+good::iron]     = fillCart( ResourceGroup::carts, 73, frontCart);
  carts[animMegaCart+good::timber]   = fillCart( ResourceGroup::carts, 81, frontCart);
  carts[animMegaCart+good::clay]     = fillCart( ResourceGroup::carts, 89, frontCart);
  carts[animMegaCart+good::marble]   = fillCart( ResourceGroup::carts, 97, frontCart);
  carts[animMegaCart+good::weapon]   = fillCart( ResourceGroup::carts, 105, frontCart);
  carts[animMegaCart+good::furniture]= fillCart( ResourceGroup::carts, 113, frontCart);
  carts[animMegaCart+good::pottery]  = fillCart( ResourceGroup::carts, 121, frontCart);*/

  VariantMap imScarbInfo = config.get( "immigrantScarb" ).toMap();
  loadAnimation( animImmigrantCart, imScarbInfo, stgCarts );
                   //fillCart( ResourceGroup::carts, 129, !frontCart);
    //carts[animImmigrantCart + 1] = fillCart( ResourceGroup::carts, 137, !frontCart);

  VariantMap circusInfo = config.get( "circusCart" ).toMap();
  loadAnimation( animCircusCart, circusInfo, stgCarts );
    //carts[animCircusCart + 0] = fillCart( ResourceGroup::carts, 601, !frontCart);
    //carts[animCircusCart + 1] = fillCart( ResourceGroup::carts, 609, !frontCart);
}

AnimationBank& AnimationBank::instance()
{
  static AnimationBank inst;
  return inst;
}

AnimationBank::AnimationBank() : _d( new Impl )
{

}

void AnimationBank::loadCarts(vfs::Path model)
{
  Logger::warning( "AnimationBank: loading cart graphics" );

  _d->loadCarts(model);
}

void AnimationBank::Impl::loadAnimation( DirectedAnimations& refMap , int who, const std::string& prefix,
                                         const int start, const int size,
                                         Walker::Action wa, const int step,
                                         int delay)
{
  MovementAnimation& ioMap = refMap[ who ].actions;
  DirectedAction action( wa, noneDirection );

  if( step == 0 )
  {
    action.direction = north;      ioMap[action].load( prefix, start,   size, Animation::straight, 1 );
  }
  else
  {
    action.direction = north;      ioMap[action].load( prefix, start,   size, Animation::straight, step); ioMap[action].setDelay( delay );
    action.direction = northEast;  ioMap[action].load( prefix, start+1, size, Animation::straight, step); ioMap[action].setDelay( delay );
    action.direction = east;       ioMap[action].load( prefix, start+2, size, Animation::straight, step); ioMap[action].setDelay( delay );
    action.direction = southEast;  ioMap[action].load( prefix, start+3, size, Animation::straight, step); ioMap[action].setDelay( delay );
    action.direction = south;      ioMap[action].load( prefix, start+4, size, Animation::straight, step); ioMap[action].setDelay( delay );
    action.direction = southWest;  ioMap[action].load( prefix, start+5, size, Animation::straight, step); ioMap[action].setDelay( delay );
    action.direction = west;       ioMap[action].load( prefix, start+6, size, Animation::straight, step); ioMap[action].setDelay( delay );
    action.direction = northWest;  ioMap[action].load( prefix, start+7, size, Animation::straight, step); ioMap[action].setDelay( delay );
  }
}

void AnimationBank::Impl::loadAnimation(int type, const VariantMap& desc, LoadingStage stage )
{  
  PictureInfoBank& pib = PictureInfoBank::instance();

  foreach( ac, desc )
  {   
    VariantMap actionInfo = ac->second.toMap();
    VARIANT_INIT_STR( rc, actionInfo )
    VARIANT_INIT_ANY( int, start, actionInfo )
    VARIANT_INIT_ANY( bool, reverse, actionInfo )
    VARIANT_INIT_ANY( int, frames, actionInfo )
    VARIANT_INIT_ANY( int, action, actionInfo )
    VARIANT_INIT_ANY( int, delay, actionInfo )
    VARIANT_INIT_ANY( int, step, actionInfo )

    switch( stage )
    {
    case stgSimple:
      {
        VARIANT_LOAD_ANY( type, actionInfo )
        Logger::warning( "AnimationBank: load simple animations for " + ac->first );
        Animation& animation = simpleAnimations[ type ];
        animation.load( rc, start, frames, reverse, step );
        animation.setDelay( delay );
      }
    break;

    case stgObjects:
      {
        //creating information about animation offset
        Point offset = pib.getDefaultOffset( PictureInfoBank::walkerOffset );
        VARIANT_LOAD_ANYDEF( offset, actionInfo, offset );
        pib.setOffset( rc, start, frames * (step == 0 ? 1 : step), offset );

        std::string typeName = WalkerHelper::getTypename( (walker::Type)type );
        Logger::warning( "AnimationBank: load animations for " + typeName + ":" + ac->first );
        loadAnimation( objects, type, rc, start, frames, (Walker::Action)action, step, delay );
      }
    break;

    case stgCarts:
      {
        //Point offset = pib.getDefaultOffset( PictureInfoBank::walkerOffset );
        //VARIANT_LOAD_ANYDEF( offset, actionInfo, offset );
        //pib.setOffset( rc, start, frames * (step == 0 ? 1 : step), offset );

        Logger::warning( "AnimationBank: load animations for %d:%s", type, ac->first.c_str() );
        loadAnimation( carts, type, rc, start, frames, Walker::acMove, step, delay );

        VARIANT_INIT_ANY( int, back, actionInfo )
        fixCartOffset( type, back );
      }
    break;
    }
  }
}

const AnimationBank::MovementAnimation& AnimationBank::Impl::tryLoadAnimations(int wtype)
{
  AnimationConfigs::iterator configIt = animConfigs.find( wtype );

  if( configIt != animConfigs.end() )
  {
    loadAnimation( wtype, configIt->second, stgObjects );
    animConfigs.erase( configIt );
  }

  DirectedAnimations::iterator it = objects.find( wtype );
  if( it == objects.end() )
  {
    Logger::warning( "WARNING !!!: AnimationBank can't find config for type %d", wtype );
    const AnimationBank::MovementAnimation& elMuleta = objects[ walker::unknown ].actions;
    objects[ wtype ].ownerType = wtype;
    objects[ wtype ].actions = elMuleta;
    return elMuleta;
  }

  return it->second.actions;
}

const AnimationBank::MovementAnimation& AnimationBank::find( int type )
{
  Impl::DirectedAnimations& dAnim = instance()._d->objects;

  Impl::DirectedAnimations::iterator it = dAnim.find( type );
  if( it == dAnim.end() )
  {
    const AnimationBank::MovementAnimation& ret = instance()._d->tryLoadAnimations( type );
    return ret;
  }

  return it->second.actions;
}

void AnimationBank::loadAnimation(vfs::Path model, vfs::Path basic)
{
  Logger::warning( "AnimationBank: start loading animations from " + model.toString() );
  _d->loadAnimation( _d->objects, 0, ResourceGroup::citizen1, 1, 12, Walker::acMove );

  VariantMap items = SaveAdapter::load( model );

  foreach( i, items )
  {
    walker::Type wtype = WalkerHelper::getType( i->first );
    if( wtype != walker::unknown )
    {
      Logger::warning( "Load config animations for " + i->first );
      _d->animConfigs[ wtype ] = i->second.toMap();
    }
    else
    {
      Logger::warning( "AnimationBank: cannot find type " + i->first );
    }
  }

  items = SaveAdapter::load( basic );
  _d->loadAnimation( animUnknown, items, Impl::stgSimple );
}

void AnimationBank::Impl::fixCartOffset( int who, bool back )
{
  /*Pictures ioCart;

  ioCart.resize(countDirection);

  ioCart[north]      = Picture::load(ResourceGroup::carts, start);
  ioCart[northEast]  = Picture::load(ResourceGroup::carts, start + 1);
  ioCart[east]       = Picture::load(ResourceGroup::carts, start + 2);
  ioCart[southEast]  = Picture::load(ResourceGroup::carts, start + 3);
  ioCart[south]      = Picture::load(ResourceGroup::carts, start + 4);
  ioCart[southWest]  = Picture::load(ResourceGroup::carts, start + 5);
  ioCart[west]       = Picture::load(ResourceGroup::carts, start + 6);
  ioCart[northWest]  = Picture::load(ResourceGroup::carts, start + 7); */

#define __CDA(a) DirectedAction(Walker::acMove,a)
  MovementAnimation& ma = carts[who].actions;
  ma[ __CDA(south) ].setOffset( back ? backCartOffsetSouth : frontCartOffsetSouth);
  ma[ __CDA(west)  ].setOffset ( back ? backCartOffsetWest  : frontCartOffsetWest );
  ma[ __CDA(north) ].setOffset( back ? backCartOffsetNorth : frontCartOffsetNorth);
  ma[ __CDA(east)  ].setOffset ( back ? backCartOffsetEast  : frontCartOffsetEast );

  ma[ __CDA(southEast) ].setOffset( back ? backCartOffsetSouthEast : frontCartOffsetSouthEast );
  ma[ __CDA(northWest) ].setOffset( back ? backCartOffsetNorthWest : frontCartOffsetNorthWest );
  ma[ __CDA(northEast) ].setOffset( back ? backCartOffsetNorthEast : frontCartOffsetNorthEast );
  ma[ __CDA(southWest) ].setOffset( back ? backCartOffsetSouthWest : frontCartOffsetSouthWest );
}

const Animation& AnimationBank::getCart(int good, int capacity, constants::Direction direction)
{
  int index = 0;
  if( capacity > animBigCart ) index = animMegaCart;
  else if( capacity > animSimpleCart ) index = animBigCart;

  MovementAnimation& ma = instance()._d->carts[ index + good ].actions;
  return ma[ DirectedAction( Walker::acMove, direction ) ];
}

const Animation& AnimationBank::simple(int type)
{
  Impl::SimpleAnimations& sAnim = instance()._d->simpleAnimations;

  Impl::SimpleAnimations::iterator it = sAnim.find( type );
  if( it != sAnim.end() )
  {
    return it->second;
  }

  return sAnim[ animUnknown ];
}

}//end namespace gfx
