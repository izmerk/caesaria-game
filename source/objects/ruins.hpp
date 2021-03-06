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

#ifndef __CAESARIA_RUINS_H_INCLUDE_
#define __CAESARIA_RUINS_H_INCLUDE_

#include "building.hpp"
#include "constants.hpp"

class Ruins : public Building
{
public:
  Ruins( object::Type type );
  void setInfo(const std::string& parent);

  virtual std::string pinfo() const;
  virtual void save(VariantMap &stream) const;
  virtual void load(const VariantMap &stream);
  virtual bool build(const city::AreaInfo &info);
  virtual void collapse() {}
  virtual void burn() {}
  virtual Variant getProperty(const std::string& name) const;
  virtual bool getMinimapColor(int &color1, int color2) const;

  void afterBuild() {_alsoBuilt=false;}
protected:
  std::string _parent;
  float _value;
  bool _alsoBuilt;
};

class BurningRuins : public Ruins
{
  static const int defaultForce = 2;
public:
  BurningRuins();

  virtual void timeStep(const unsigned long time);
  virtual bool build(const city::AreaInfo &info);
  virtual bool isWalkable() const;
  virtual bool isDestructible() const;
  virtual void destroy();
  virtual bool isFlat() const { return false; }
  virtual bool canDestroy() const;
  virtual bool getMinimapColor(int& color1, int& color2) const;

  virtual float evaluateService( ServiceWalkerPtr walker);
  virtual void applyService( ServiceWalkerPtr walker);
  virtual bool isNeedRoad() const;
};

class BurnedRuins : public Ruins
{
public:
  BurnedRuins();

  virtual void timeStep(const unsigned long time);
  virtual bool isWalkable() const;
  virtual bool isFlat() const;
  virtual bool build(const city::AreaInfo &info);
  virtual bool isNeedRoad() const;
  virtual void destroy();
};

typedef SmartPtr< BurningRuins > BurningRuinsPtr;
typedef SmartPtr< BurnedRuins > BurnedRuinsPtr;

class CollapsedRuins : public Ruins
{
public:
  CollapsedRuins();

  virtual bool build(const city::AreaInfo &info);

  virtual bool isWalkable() const;
  virtual bool isFlat() const;
  virtual bool isNeedRoad() const;
};

class PlagueRuins : public Ruins
{
public:
  PlagueRuins();

  virtual void timeStep(const unsigned long time);
  virtual bool isDestructible() const;
  virtual bool build( const city::AreaInfo& info );
  virtual bool isWalkable() const;
  virtual void destroy();

  virtual void applyService(ServiceWalkerPtr walker);

  virtual bool isNeedRoad() const;
};

#endif //__CAESARIA_RUINS_H_INCLUDE_
