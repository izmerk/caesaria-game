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
// Copyright 2012-2014 Dalerank, dalerankn8@gmail.com

#ifndef _CAESARIA_SOUND_ENGINE_H_INCLUDE_
#define _CAESARIA_SOUND_ENGINE_H_INCLUDE_

#include <string>

#include "vfs/path.hpp"
#include "core/variant.hpp"
#include "constants.hpp"
#include "config.hpp"
#include "core/signals.hpp"
#include "core/singleton.hpp"

namespace audio
{

class Engine : public StaticSingleton<Engine>
{
  SET_STATICSINGLETON_FRIEND_FOR(Engine)
public:
  void setVolume( SoundType type, Volume value);
  void loadAlias(const vfs::Path& filename );
  void addFolder( vfs::Directory dir );

  Volume volume(SoundType type) const;

  int maxVolumeValue() const;

  ~Engine();

  void init();
  void exit();

  void play(std::string sampleName, int volume, SoundType type, bool force=false);
  void play(const std::string& rc, int index, int volume, SoundType type, bool force=false);

  bool isPlaying(const std::string& sampleName) const;

  void stop(const std::string& sampleName) const;
  void stop(int channel);

  void run(bool&);
public signals:
  Signal0<>& onThemeStopped();

private:
  Engine();
  void _updateSamplesVolume();

  class Impl;
  ScopedPtr< Impl > _d;
};

class Helper
{
public:
  static void initTalksArchive(const vfs::Path& filename );
};

} //end namespace audio

#endif //_CAESARIA_SOUND_ENGINE_H_INCLUDE_

