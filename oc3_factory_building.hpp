// This file is part of openCaesar3.
//
// openCaesar3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openCaesar3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openCaesar3.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012-2013 Gregoire Athanase, gathanase@gmail.com


#ifndef FACTORY_BUILDING_HPP
#define FACTORY_BUILDING_HPP

#include "oc3_building.hpp"
#include "oc3_predefinitions.hpp"

class Factory: public WorkingBuilding
{
public:
   Factory( const GoodType inGood, const GoodType outGood,
            const BuildingType type, const Size& size );
  ~Factory();

   GoodStock& getInGood();
   GoodStock& getOutGood();
   SimpleGoodStore& getGoodStore();

   // called when the factory has made 100 good units
   void deliverGood();
   int getProgress();

   virtual void timeStep(const unsigned long time);

   virtual GuiInfoBox* makeInfoBox( Widget* parent );

   void serialize(OutputSerialStream &stream);
   void unserialize(InputSerialStream &stream);

   void removeWalker( WalkerPtr w );
protected:
   virtual bool _mayDeliverGood() const;
   void _addWalker( WalkerPtr );

protected:
   GoodType _inGoodType;
   GoodType _outGoodType;
   SimpleGoodStore _goodStore;

   float _productionRate;  // max production / year
   float _progress;  // progress of the work, in percent (0-100).
   Picture *_stockPicture; // stock of input good

   class Impl;
   ScopedPtr< Impl > _d;
};


class FactoryMarble : public Factory
{
public:
   FactoryMarble();

   bool canBuild(const TilePos& pos ) const;  // returns true if it can be built there
   void timeStep(const unsigned long time);
};

class FactoryTimber : public Factory
{
public:
   FactoryTimber();
   bool canBuild(const TilePos& pos ) const;  // returns true if it can be built there
};

class FactoryIron : public Factory
{
public:
   FactoryIron();
   bool canBuild(const TilePos& pos ) const;  // returns true if it can be built there
};

class FactoryWeapon : public Factory
{
public:
   FactoryWeapon();
};

class FactoryFurniture : public Factory
{
public:
   FactoryFurniture();
};

class FactoryWine : public Factory
{
public:
   FactoryWine();
};

class FactoryOil : public Factory
{
public:
   FactoryOil();
};

class FactoryPottery : public Factory
{
public:
   FactoryPottery();
};


class FarmTile
{
public:
   FarmTile(const GoodType outGood, const TilePos& pos );
   void computePicture(const int percent);
   Picture& getPicture();

private:
   int _i;
   int _j;
   Picture _picture;
   Animation _animation;
};


class Farm : public Factory
{
public:
   Farm(const GoodType outGood, const BuildingType type );
   void init();

   void computePictures();
   virtual void timeStep(const unsigned long time);
   virtual bool canBuild(const TilePos& pos ) const;  // returns true if it can be built there
   
protected:
   std::vector<FarmTile> _subTiles;
   Picture _pictureBuilding;  // we need to change its offset
};

class FarmWheat : public Farm
{
public:
   FarmWheat();
};

class FarmOlive : public Farm
{
public:
   FarmOlive();
};

class FarmGrape : public Farm
{
public:
   FarmGrape();
};

class FarmMeat : public Farm
{
public:
   FarmMeat();
};

class FarmFruit : public Farm
{
public:
   FarmFruit();
};

class FarmVegetable : public Farm
{
public:
  FarmVegetable();
};

class Wharf : public Factory
{
public:
  Wharf();
  virtual bool canBuild(const TilePos& pos ) const;  // returns true if it can be built there
};

#endif
