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
// Copyright 2012-2016 Dalerank, dalerankn8@gmail.com

#include "listbox.hpp"
#include "listboxprivate.hpp"
#include "pushbutton.hpp"
#include "core/time.hpp"
#include "core/utils.hpp"
#include "core/event.hpp"
#include "core/variant_map.hpp"
#include "core/variant_list.hpp"
#include "gfx/engine.hpp"
#include "gfx/decorator.hpp"
#include "gfx/drawstate.hpp"
#include "core/logger.hpp"
#include "core/gettext.hpp"
#include "widget_factory.hpp"

#define DEFAULT_SCROLLBAR_SIZE 39

using namespace gfx;

namespace gui
{

REGISTER_CLASS_IN_WIDGETFACTORY(ListBox)

//! constructor
ListBox::ListBox( Widget* parent,const Rect& rectangle,
      int id, bool clip,
      bool drawBack, bool moveOverSel)
: Widget( parent, id, rectangle),
  _d( new Impl )
{
  _d->dragEventSended = false;
  _d->index.hovered = -1;
  _d->height.item = 0;
  _d->height.total = 0;
  _d->height.override = 0;
  _d->font = Font();
  _d->itemsIconWidth = 0;
  _d->scrollBar = 0;
  _d->color.text = ColorList::black;
  _d->color.textHighlight = NColor(0xffe0e0e0);
  _d->time.select = 0;
  _d->index.selected = -1;
  _d->time.lastKey = 0;
  _d->selecting = false;
  _d->needItemsRepackTextures = true;

#ifdef _DEBUG
  setDebugName( "ListBox");
#endif

  setFlag(selectOnMove, false);
  setFlag(moveOverSelect, moveOverSel);
  setFlag(autoscroll, true);
  setFlag(hightlightNotinfocused, true);
  setFlag(itemSelectable, true);
  setFlag(drawBackground, drawBack);

  const int s = DEFAULT_SCROLLBAR_SIZE;

  _d->scrollBar = &add<ScrollBar>(Rect( width() - s, 0, width(), height()), false);
  _d->scrollBar->setNotClipped( false );
  _d->scrollBar->setSubElement(true);
  _d->scrollBar->setVisibleFilledArea( false );
  _d->scrollBar->setTabstop(false);
  _d->scrollBar->setAlignment( align::lowerRight, align::lowerRight, align::upperLeft, align::lowerRight);
  _d->scrollBar->setVisible(false);
  _d->scrollBar->setValue(0);

  setNotClipped(!clip);

  // this element can be tabbed to
  setTabstop(true);
  setTaborder(-1);

  updateAbsolutePosition();

  setTextAlignment( align::upperLeft, align::center );
  _recalculateItemHeight( Font::create( "FONT_2" ), height() );
}

ListBox::ListBox(Widget* parent, const RectF& rectangle, int id, bool clip, bool drawBack, bool mos)
  : ListBox( parent, Rect( 0, 0, 1, 1), id, clip, drawBack, mos)
{
  setGeometry(rectangle);
}

void ListBox::_recalculateItemHeight( const Font& defaulFont, int h )
{
  if( !_d->font.isValid() )
  {
    _d->font = defaulFont;

    if ( _d->height.override != 0 )
      _d->height.item = _d->height.override;
    else
      _d->height.item = _d->font.getTextSize("A").height() + 4;
  }

  int newLength = _d->height.item * _d->items.size();
  bool scrollBarVisible = _d->scrollBar->visible();

  if( newLength != _d->height.total )
  {
    _d->height.total = newLength;
    _d->scrollBar->setMaxValue( std::max<int>( 0, _d->height.total - h ) );

    int minItemHeight = _d->height.item > 0 ? _d->height.item : 1;
    _d->scrollBar->setSmallStep ( minItemHeight );
    _d->scrollBar->setLargeStep ( 2*minItemHeight );

    _d->scrollBar->setVisible( !( _d->height.total <= h ) );
  }

  if( scrollBarVisible != _d->scrollBar->visible() )
  {
    _updateBackground( _d->scrollBar->visible() ? _d->scrollBar->width() : 0 );
  }
}

//! destructor
ListBox::~ListBox() {}

//! returns amount of list items
unsigned int ListBox::itemsCount() const {  return _d->items.size(); }

//! returns string of a list item. the may be a value from 0 to itemCount-1
ListBoxItem& ListBox::itemAt(unsigned int index)
{
  if(index >= _d->items.size() ) {
    Logger::warning( "Index out of range ListBox::items [{0}]", index);
    return ListBoxItem::invalidItem();
  }

  return _d->items[index];
}

ListBoxItem& ListBox::selectedItem() {	return itemAt( selected() ); }

//! adds a list item, returns id of item
void ListBox::removeItem(unsigned int id)
{
  if (id >= _d->items.size())
  {
    return;
  }

  if( (unsigned int)_d->index.selected==id )
  {
    _d->index.selected = -1;
  }
  else if ((unsigned int)_d->index.selected > id)
  {
    _d->index.selected -= 1;
    _d->time.select = DateTime::elapsedTime();
  }

  _d->items.erase( _d->items.begin() + id);

  _recalculateItemHeight( _d->font, height() );
}

int ListBox::findIndex(Point pos) const
{
  if(	pos.x() < screenLeft() || pos.x() >= screenRight()
       ||	pos.y() < screenTop() || pos.y() >= screenBottom() )
  {
    return -1;
  }

  if ( _d->height.item == 0 )
  {
    return -1;
  }

  int item = ((pos.y() - screenTop() - 1) + _d->scrollBar->value()) / _d->height.item;

  if ( item < 0 || item >= (int)_d->items.size())
  {
    return -1;
  }

  return item;
}

//! clears the list
void ListBox::clear()
{
  _d->items.clear();
  _d->itemsIconWidth = 0;
  _d->index.selected = -1;

  if (_d->scrollBar)
  {
    _d->scrollBar->setValue(0);
  }

  _recalculateItemHeight( _d->font, height() );
}

//! sets the selected item. Set this to -1 if no item should be selected
void ListBox::setSelected(int id)
{
  id = isFlag( itemSelectable )
        ? ((unsigned int)id>=_d->items.size() ? -1 : id)
        : -1;
  _d->index.selected = id;

  _d->time.select = DateTime::elapsedTime();
  _d->needItemsRepackTextures = true;

  _recalculateScrollPos();
}

int ListBox::findItem(const std::string& text) const
{
  int index = 0;
  for( auto& it : _d->items )
  {
    if( it.text() == text )
      return index;

    index++;
  }

  return -1;
}

void ListBox::setSelectedTag(const Variant& tag)
{
  int index = 0;
  for (auto& it : _d->items)
  {
    if (it.tag() == tag)
    {
      setSelected( index );
      break;
    }
    index++;
  }
}

void ListBox::setSelectedWithData(const std::string& name, const Variant& data)
{
  int index = 0;
  for (auto& it : _d->items)
  {
    if( it.data(name) == data )
    {
      setSelected(index);
      break;
    }
    index++;
  }
}

//! sets the selected item. Set this to -1 if no item should be selected
void ListBox::setSelected(const std::string& item)
{
  int index = -1;

  for (index = 0; index < (int) _d->items.size(); ++index)
  {
    if (_d->items[index].text() == item)
      break;
  }

  setSelected(index);
}

void ListBox::_indexChanged(unsigned int eventType)
{
  parent()->onEvent( NEvent::ev_gui( this, 0, (event::gui::Type)eventType ) );

  switch( eventType )
  {
  case event::gui::listboxChanged:
  {
    emit _d->signal.onIndexSelected(_d->index.selected);
    if( _d->index.selected >= 0 )
    {
      emit _d->signal.onTextSelected( _d->items[ _d->index.selected ].text() );
      emit _d->signal.onItemSelected( _d->items[ _d->index.selected ] );
      emit _d->signal.onIndexSelectedEx( this, _d->index.selected );
    }
  }
  break;

  case event::gui::listboxSelectedAgain:
  {
    emit _d->signal.onIndexSelectedAgain( _d->index.selected );
    if( _d->index.selected >= 0 )
    {
      emit _d->signal.onItemSelectedAgain( _d->items[ _d->index.selected ] );
      emit _d->signal.onIndexSelectedAgainEx( this, _d->index.selected);
    }
  }
  break;

  default:
  break;
  }
}

//! called if an event happened.
bool ListBox::onEvent(const NEvent& event)
{
  if( enabled() )
  {
    switch(event.EventType)
    {
    case sEventMax:
    case sEventUser:
    break;

    case sEventKeyboard:
      if (event.keyboard.pressed &&
        (event.keyboard.key == KEY_DOWN ||
        event.keyboard.key == KEY_UP   ||
        event.keyboard.key == KEY_HOME ||
        event.keyboard.key == KEY_END  ||
        event.keyboard.key == KEY_NEXT ||
        event.keyboard.key == KEY_PRIOR ) )
      {
        int oldSelected = _d->index.selected;
        switch (event.keyboard.key)
        {
          case KEY_DOWN: _d->index.selected += 1; break;
          case KEY_UP:   _d->index.selected -= 1; break;
          case KEY_HOME: _d->index.selected = 0;  break;
          case KEY_END:  _d->index.selected = (int)_d->items.size()-1; break;
          case KEY_NEXT: _d->index.selected += height() / _d->height.item; break;
          case KEY_PRIOR:_d->index.selected -= height() / _d->height.item; break;
          default: break;
        }

        math::clamp_to<int>( _d->index.selected, 0, _d->items.size() - 1 );

        _recalculateScrollPos();
        _d->needItemsRepackTextures = true;

        // post the news
        if( oldSelected != _d->index.selected && !_d->selecting && !isFlag( moveOverSelect ) )
        {
          _indexChanged( event::gui::listboxChanged );
        }

        return true;
      }
      else if (!event.keyboard.pressed && ( event.keyboard.key == KEY_RETURN || event.keyboard.key == KEY_SPACE ) )
      {
        _indexChanged( event::gui::listboxSelectedAgain );

        return true;
      }
      else if (event.keyboard.pressed && event.keyboard.symbol)
      {
        // change selection based on text as it is typed.
        unsigned int now = DateTime::elapsedTime();

        if (now - _d->time.lastKey < 500)
        {
          // add to key buffer if it isn't a key repeat
          if (!(_d->keyBuffer.size() == 1 && _d->keyBuffer[0] == event.keyboard.symbol))
          {
            _d->keyBuffer += " ";
            _d->keyBuffer[_d->keyBuffer.size()-1] = event.keyboard.symbol;
          }
        }
        else
        {
          _d->keyBuffer = " ";
          _d->keyBuffer[0] = event.keyboard.symbol;
        }
        _d->time.lastKey = now;

        // find the selected item, starting at the current selection
        int start = _d->index.selected;

        // dont change selection if the key buffer matches the current item
        if (_d->index.selected > -1 && _d->keyBuffer.size() > 1)
        {
          if( _d->items[ _d->index.selected ].text().size() >= _d->keyBuffer.size()
              && utils::isEquale( _d->keyBuffer, _d->items[_d->index.selected].text().substr( 0,_d->keyBuffer.size() ),
                                        utils::equaleIgnoreCase ) )
          {
            return true;
          }
        }

        int current;
        for( current = start+1; current < (int)_d->items.size(); ++current)
        {
          if( _d->items[current].text().size() >= _d->keyBuffer.size())
          {
            if( utils::isEquale( _d->keyBuffer, _d->items[current].text().substr(0,_d->keyBuffer.size()),
                                        utils::equaleIgnoreCase ) )
            {
              if ( _d->index.selected != current && !_d->selecting && !isFlag( moveOverSelect ))
              {
                _indexChanged( event::gui::listboxChanged );
              }

              setSelected(current);
              return true;
            }
          }
        }

        for( current = 0; current <= start; ++current)
        {
          if( _d->items[current].text().size() >= _d->keyBuffer.size())
          {
            if( utils::isEquale( _d->keyBuffer, _d->items[current].text().substr( 0,_d->keyBuffer.size() ),
                                        utils::equaleIgnoreCase ) )
            {
              if ( _d->index.selected != current && !_d->selecting && !isFlag( moveOverSelect ))
              {
                _indexChanged( event::gui::listboxChanged );
              }

              setSelected(current);
              return true;
            }
          }
        }

        return true;
      }
      break;

    case sEventGui:
      switch(event.gui.type)
      {
      case event::gui::scrollbarChanged:
      {
        if (event.gui.caller == _d->scrollBar)
        {
          _d->needItemsRepackTextures = true;
          return true;
        }
      }
      break;

      case event::gui::widget::focused:
      break;

      case event::gui::widget::focusLost:
      {
        if (event.gui.caller == this)
        {
           _d->selecting = false;
        }
      }
      break;

      default:
      break;
      }
      break;

      case sEventMouse:
      {
        Point p = event.mouse.pos();

        switch(event.mouse.type)
        {
        case NEvent::Mouse::mouseWheel:
        {
          _d->scrollBar->setValue(_d->scrollBar->value() + (event.mouse.wheel < 0 ? -1 : 1) * (-_d->height.item/2));
          _d->needItemsRepackTextures = true;
          return true;
        }
        break;

        case NEvent::Mouse::mouseRbtnRelease:
        {
          if (isPointInside(p)) {
            int index = findIndex(event.mouse.pos());
            emit _d->signal.onIndexRmbClickedEx(this, index);
          }
        }
        break;

        case NEvent::Mouse::btnLeftPressed:
        {
          _d->dragEventSended = false;
          _d->selecting = true;

          if (isPointInside(p) && isFlag(selectOnMDown)) {
            _selectNew(event.mouse.y);
          }

          return true;
        }
        break;

        case NEvent::Mouse::mouseLbtnRelease:
        {
          _d->selecting = false;

          if (isPointInside(p) && !isFlag(selectOnMDown)) {
            _selectNew(event.mouse.y);
          }

          return true;
        }
        break;

        case NEvent::Mouse::moved:
        {
          if (_d->selecting && isFlag(selectOnMove)) {
            if (isPointInside(p)) {
              _selectNew(event.mouse.y);
              return true;
            }
          }

        }
        break;

        default:
        break;
        }
      }
      break;

      default: break;
    }
  }

  return Widget::onEvent(event);
}

void ListBox::_selectNew(int ypos)
{
  unsigned int now = DateTime::elapsedTime();
  int oldSelected = _d->index.selected;

  _d->needItemsRepackTextures = true;

  int newIndex = findIndex({screenLeft(), ypos});
  ListBoxItem& ritem = itemAt(newIndex);

  if( ritem.isEnabled() )
  {
    _d->index.selected = newIndex;
    if( _d->index.selected<0 && !_d->items.empty() )
        _d->index.selected = 0;

    _recalculateScrollPos();

    auto eventType = ( _d->index.selected == oldSelected && now < _d->time.select + 500)
                                   ? event::gui::listboxSelectedAgain
                                   : event::gui::listboxChanged;
    _d->time.select = now;
    // post the news
    _indexChanged( eventType );
  }
}

//! Update the position and size of the listbox, and update the scrollbar
void ListBox::_finalizeResize()
{
  _d->height.total = 0;
  _recalculateItemHeight( _d->font, height() );
  _updateBackground( _d->scrollBar->visible() ? _d->scrollBar->width() : 0 );
}

ElementState ListBox::_getCurrentItemState( unsigned int index, bool hl )
{
  if( _d->items[ index ].isEnabled() )
  {
    if( hl && (int)index == _d->index.selected )
      return stChecked;

    if( (int)index == _d->index.selected )
      return stHovered;

    return stNormal;
  }

  return stDisabled;
}

Font ListBox::_getCurrentItemFont( const ListBoxItem& item, bool selected )
{
  Font itemFont = item.overrideColors[ selected ? ListBoxItem::hovered : ListBoxItem::simple ].font;

  if( !itemFont.isValid() )
      itemFont = _d->font;

  return itemFont;
}

NColor ListBox::_getCurrentItemColor( const ListBoxItem& item, bool selected )
{
  NColor ret = ColorList::clear;
  ListBoxItem::ColorType tmpState = selected ? ListBoxItem::hovered : ListBoxItem::simple;

  if (item.overrideColors[ tmpState ].Use)
    ret = item.overrideColors[ tmpState ].color;
  else if (ret == ColorList::clear)
    ret = itemDefaultColor( tmpState );

  return ret;
}

Rect ListBox::_itemsRect()
{
  Rect frameRect( Point( 0, 0 ), size() );

  frameRect.rright() = frameRect.right() - DEFAULT_SCROLLBAR_SIZE;

  return frameRect;
}

void ListBox::_drawItemIcon( Engine& painter, ListBoxItem& item, const Point& pos, Rect* clipRect)
{
  painter.draw( item.icon(), pos + item.iconOffset(), clipRect );
}

void ListBox::_drawItemText( Engine& painter, ListBoxItem& item, const Point& pos, Rect* clipRect )
{
  painter.draw( item.picture(), pos + item.textOffset(), clipRect );
}

void ListBox::_updateItemText(Engine& painter, ListBoxItem& item, const Rect& textRect, Font font, const Rect& frameRect)
{
  item.updateText( textRect.lefttop(), font, frameRect.size() );
}

void ListBox::beforeDraw(gfx::Engine& painter)
{
  if ( !visible() )
      return;

  if( _d->needItemsRepackTextures )
  {
    bool hl = ( isFlag( hightlightNotinfocused ) || isFocused() || _d->scrollBar->isFocused() );
    Rect frameRect = _itemsRect();
    frameRect.rbottom() = frameRect.top() + _d->height.item;

    Alignment itemTextHorizontalAlign, itemTextVerticalAlign;
    Font currentFont;

    for( size_t i = 0; i < _d->items.size();  i++ )
    {
      ListBoxItem& refItem = _d->items[ i ];

      if( refItem.icon().isValid() )
      {
        if( refItem.horizontalAlign() == align::center )
        {
          Point offset( (width() - refItem.icon().width()) / 2, refItem.iconOffset().y() );
          refItem.setIconOffset( offset );
        }
      }

      int mnY = frameRect.bottom() - _d->scrollBar->value();
      int mxY = frameRect.top() - _d->scrollBar->value();
      if( !refItem.text().empty() && mnY >= 0 && mxY <= (int)height() )
      {
        bool underMouse = ( static_cast<int>(i) == _d->index.selected && hl);
        refItem.setState( _getCurrentItemState( i, hl ) );

        itemTextHorizontalAlign = refItem.isAlignEnabled() ? refItem.horizontalAlign() : horizontalTextAlign();
        itemTextVerticalAlign = refItem.isAlignEnabled() ? refItem.verticalAlign() : verticalTextAlign();

        currentFont = _getCurrentItemFont( refItem, underMouse );
        currentFont.setColor( _getCurrentItemColor( refItem, underMouse ) );

        Point p;
        if (refItem.icon().isValid()) {
          p = Point(refItem.icon().width(), 0 );
        }
        Rect textRect = currentFont.getTextRect( refItem.text(), Rect( p, frameRect.size() ),
                                                 itemTextHorizontalAlign, itemTextVerticalAlign );

        textRect._lefttop += Point( _d->itemsIconWidth+3, 0 );

        _updateItemText( painter, refItem, textRect, currentFont, frameRect);
      }

      frameRect += Point( 0, _d->height.item );
    }

    _d->needItemsRepackTextures = false;
  }

  Widget::beforeDraw( painter );
}

void ListBox::refresh() {  _d->needItemsRepackTextures = true; }

//! draws the element and its children
void ListBox::draw( gfx::Engine& painter )
{
  if ( !visible() )
    return;

  if (isFlag(drawBackground))
  {
    DrawState pipe( painter, absoluteRect().lefttop(), &absoluteClippingRectRef() );
    pipe.draw( _d->bg.batch )
        .fallback( _d->bg.fallback );
  }

  Point scrollBarOffset( 0, -_d->scrollBar->value() );
  Rect frameRect = _itemsRect();
  frameRect += _d->margin.lefttop();
  frameRect.rbottom() = frameRect.top() + _d->height.item;
  const Point& widgetLeftup = absoluteRect().lefttop();

  Rect clipRect = absoluteClippingRectRef();
  clipRect._lefttop += Point( 3, 3 );
  clipRect._bottomright -= Point( 3, 3 );

  for (auto& refItem :  _d->items) {
    int mnY = frameRect.bottom() - _d->scrollBar->value();
    int mxY = frameRect.top() - _d->scrollBar->value();

    mnY += std::max( 0, refItem.icon().height() - frameRect.height() );

    bool overBorder = (mnY < 0 && mxY < 0) || (mnY > (int)height() && mxY > (int)height());
    if( !overBorder ) {
      if (refItem.icon().isValid()) {
        _drawItemIcon( painter, refItem, widgetLeftup + frameRect.lefttop() + scrollBarOffset, &clipRect );
      }

      if (refItem.picture().isValid()) {
        _drawItemText( painter, refItem, widgetLeftup + frameRect.lefttop() + scrollBarOffset + refItem.iconOffset(), &clipRect  );
      }
    }

    frameRect += Point( 0, _d->height.item );
  }

  Widget::draw( painter );
}

void ListBox::_recalculateScrollPos()
{
  if (!isFlag( autoscroll ))
    return;

  const int selPos = (_d->index.selected == -1 ? _d->height.total : _d->index.selected * _d->height.item) - _d->scrollBar->value();

  if (selPos < 0)
  {
    _d->scrollBar->setValue( _d->scrollBar->value() + selPos );
  }
  else if (selPos > (int)height() - _d->height.item)
  {
    _d->scrollBar->setValue( _d->scrollBar->value() + selPos - height() + _d->height.item );
  }
}

void ListBox::_updateBackground(int scrollbarWidth)
{
  _d->bg.batch.destroy();

  Pictures pics;

  Decorator::draw( pics, Rect( 0, 0, width() - scrollbarWidth, height() ), Decorator::blackFrame );
  Decorator::draw( pics, Rect( width() - scrollbarWidth, 0, width(), height() ), Decorator::whiteArea, nullptr, Decorator::normalY  );

  bool batchOk = _d->bg.batch.load( pics, absoluteRect().lefttop() );
  if( !batchOk )
  {
    _d->bg.batch.destroy();
    Decorator::reverseYoffset( pics );
    _d->bg.fallback = pics;
  }
}

void ListBox::setAutoScrollEnabled(bool scroll) {	setFlag( autoscroll, scroll );}
bool ListBox::isAutoScrollEnabled() const{	return isFlag( autoscroll );}

void ListBox::setScrollbarVisible(bool visible)
{
  if (_d->scrollBar)
    _d->scrollBar->setVisible(visible);
}

void ListBox::setItemText(unsigned int index, const std::string& text)
{
  itemAt(index).setText(text);
  _d->needItemsRepackTextures = true;
  _recalculateItemHeight(_d->font, height());
}

void ListBox::setItemIcon(unsigned int index, Picture icon, const Point& offset)
{
  itemAt(index).setIcon(icon);
  if (offset.y() > -100)
    itemAt(index).setIconOffset(offset);
}

void ListBox::setItemData(unsigned int index, const std::string& name, Variant tag)
{
  itemAt(index).setData( name, tag );
}

Variant ListBox::getItemData(unsigned int index, const std::string& name)
{
  if (index >= _d->items.size())
    return Variant();

  return _d->items[index].data(name);
}

Rect ListBox::getItemRectangle(unsigned int index)
{
  return Rect();
}

//! Insert the item at the given index
//! Return the index on success or -1 on failure.
int ListBox::insertItem(unsigned int index, std::string text)
{
  ListBoxItem i;
  i.setText( text );

  _d->items.insert( _d->items.begin() + index, i );

  _recalculateItemHeight( _d->font, height() );

  return index;
}

void ListBox::swapItems(unsigned int index1, unsigned int index2)
{
  if ( index1 >= _d->items.size() || index2 >= _d->items.size() )
    return;

  ListBoxItem dummmy = _d->items[index1];
  _d->items[index1] = _d->items[index2];
  _d->items[index2] = dummmy;
}

void ListBox::setItemOverrideColor(unsigned int index, NColor color, int colorType)
{
  if ( index >= _d->items.size() || colorType < 0 || colorType >= ListBoxItem::count )
        return;

  if( colorType == ListBoxItem::all ) {
    for ( unsigned int c=0; c < ListBoxItem::count; ++c ) {
      _d->items[index].setTextColor( ListBoxItem::ColorType(c), color);
    }
  } else {
    _d->items[index].setTextColor(ListBoxItem::ColorType(colorType), color);
  }
}

void ListBox::resetItemOverrideColor(unsigned int index)
{
  for (unsigned int c=0; c < (unsigned int)ListBoxItem::count; ++c )
  {
    _d->items[index].overrideColors[c].Use = false;
  }
}

void ListBox::setItemEnabled(unsigned int index, bool enabled)
{
  itemAt(index).setEnabled(enabled);
}

void ListBox::resetItemOverrideColor(unsigned int index, ListBoxItem::ColorType colorType)
{
  if ( index >= _d->items.size() || colorType < 0 || colorType >= ListBoxItem::count )
    return;

  _d->items[index].overrideColors[colorType].Use = false;
}


bool ListBox::hasItemOverrideColor(unsigned int index, ListBoxItem::ColorType colorType) const
{
  if ( index >= _d->items.size() || colorType < 0 || colorType >= ListBoxItem::count )
    return false;

  return _d->items[index].overrideColors[colorType].Use;
}

NColor ListBox::getItemOverrideColor(unsigned int index, ListBoxItem::ColorType colorType) const
{
  if ( (unsigned int)index >= _d->items.size() || colorType < 0 || colorType >= ListBoxItem::count )
    return ColorList::clear;

  return _d->items[index].overrideColors[colorType].color;
}

NColor ListBox::itemDefaultColor( ListBoxItem::ColorType colorType) const
{
  switch ( colorType )
  {
    case ListBoxItem::simple:
      return _d->color.text;
    case ListBoxItem::hovered:
      return _d->color.textHighlight;
    case ListBoxItem::iconSimple:
      return ColorList::white;
    case ListBoxItem::iconHovered:
      return NColor(0xff0f0f0f);
    default:
      return ColorList::clear;
  }
}

void ListBox::setItemDefaultColor(ListBoxItem::ColorType colorType, const NColor& color )
{
  switch(colorType)
  {
  case ListBoxItem::simple: _d->color.text = color; break;
  case ListBoxItem::hovered: _d->color.textHighlight = color; break;
  default: break;
  }
}

void ListBox::setItemDefaultColor(const std::string& typeName, const std::string& colorName)
{
  auto type = ListBoxItem::findColorType(typeName);
  auto color = ColorList::find(colorName);
  setItemDefaultColor(type, color);
}

void ListBox::setItemsHeight( int height )
{
  _d->height.item = height;
  _d->height.override = 1;
}

int ListBox::itemsHeight() const { return _d->height.item; }

void ListBox::setItemAlignment(int index, Alignment horizontal, Alignment vertical)
{
  itemAt(index).setTextAlignment( horizontal, vertical );
  _d->needItemsRepackTextures = true;
}

ListBoxItem& ListBox::addItem( const std::string& text, Font font, NColor color )
{
  ListBoxItem i;
  i.setText( text );
  i.setState( stNormal );
  i.setTextOffset( _d->itemTextOffset );
  i.overrideColors[ListBoxItem::simple].font = font.isValid() ? font : _d->font;
  i.overrideColors[ListBoxItem::simple].color = color;
  i.setTextAlignment( horizontalTextAlign(), verticalTextAlign() );

  _d->needItemsRepackTextures = true;
  _d->items.push_back(i);
  _recalculateItemHeight( _d->font, height() );

  return _d->items.back();
}

ListBoxItem& ListBox::addItem(Picture pic)
{
  ListBoxItem& item = addItem( "", Font() );
  item.setIcon( pic  );

  return item;
}

int ListBox::addLine(const std::string& text)
{
  addItem(text);
  return itemsCount()-1;
}

void ListBox::fitText(const std::string& text)
{
  if (text.substr(0, 5) == "@img=")
  {
    Picture pic(text.substr(5));
    ListBoxItem& item = addItem(pic);
    item.setTextAlignment( align::center, align::upperLeft );
    int lineCount = pic.height() / itemsHeight();
    StringArray lines;
    lines.resize(lineCount);
    addLines(lines);
  }
  else
  {
    StringArray items = _d->font.breakText( text, width() - _d->scrollBar->width() );
    addLines(items);
  }
}

void ListBox::addLines(const StringArray& strings)
{
  for (auto& line : strings)
  {
    if( line.find( "\tc" ) != std::string::npos )
    {
      std::string nLine = utils::replace( line, "\tc", "" );
      ListBoxItem& item = addItem( nLine );
      item.setTextAlignment( align::center, align::center );
    }
    else
    {
      addItem( line );
    }
  }
}

void ListBox::setMargin(int type, int value)
{
  switch(type)
  {
  case margin::left: _d->margin.setLeft(value);       break;
  case margin::top: _d->margin.setTop(value);         break;
  case margin::right: _d->margin.setRight(value);     break;
  case margin::bottom: _d->margin.setBottom(value);   break;
  default: break;
  }

  _d->needItemsRepackTextures = true;
}

Font ListBox::font() const{  return _d->font;}
void ListBox::setBackgroundVisible(bool draw) { setFlag( drawBackground, draw );} //! Sets whether to draw the background
int ListBox::selected() {    return _d->index.selected; }
Signal1<const ListBoxItem&>& ListBox::onItemSelectedAgain(){  return _d->signal.onItemSelectedAgain;}
Signal1<const ListBoxItem&>& ListBox::onItemSelected(){  return _d->signal.onItemSelected;}

Signal2<Widget*, int>& ListBox::onIndexSelectedEx() { return _d->signal.onIndexSelectedEx; }
Signal2<Widget*, int>& ListBox::onIndexSelectedAgainEx() { return _d->signal.onIndexSelectedAgainEx; }
Signal2<Widget*, int>&ListBox::onIndexRmbClickedEx() { return _d->signal.onIndexRmbClickedEx; }
void ListBox::setItemsFont(Font font) { _d->font = font; }
void ListBox::setItemsFont(const std::string& fname) { _d->font = Font::create(fname); }
void ListBox::setItemsTextOffset(Point p) { _d->itemTextOffset = p; }
void ListBox::setItemsSelectable(bool en) {  setFlag( itemSelectable, en ); }

void ListBox::setItemTooltip(unsigned int index, const std::string& text)
{
  itemAt(index).setTooltip(text);
}

void ListBox::setupUI(const VariantMap& ui)
{
  Widget::setupUI(ui);

  int itemheight = ui.get("itemheight");
  if (itemheight != 0)
    setItemsHeight(itemheight);

  setBackgroundVisible(ui.get("border.visible", true));
  std::string fontname = ui.get( "itemfont" ).toString();
  if (!fontname.empty())
    setItemsFont( Font::create( fontname ) );

  fontname = ui.get( "items.font" ).toString();
  if( !fontname.empty() ) setItemsFont( Font::create( fontname ) );

  Variant itemtextoffset = ui.get( "items.offset" );
  if( itemtextoffset.isValid() )
    setItemsTextOffset( itemtextoffset.toPoint() );

  _d->margin.rleft() = ui.get( "margin.left", _d->margin.left() );
  _d->margin.rtop() = ui.get( "margin.top", _d->margin.top() );

  bool scrollBarVisible = ui.get("scrollbar.visible", true);
  _d->scrollBar->setVisible(scrollBarVisible);

  VariantList items = ui.get( "items" ).toList();
  for( auto& item : items )
  {
    VariantMap vm = item.toMap();
    if( vm.empty() )
    {
      addItem( item.toString() );
    }
    else
    {
      std::string fontName = vm.get( "font" ).toString();
      std::string text = vm.get( "text" ).toString();
      int tag = vm.get( "tag" );
      Font f = fontName.empty() ? font() : Font::create( fontName );
      ListBoxItem& item = addItem( _(text), f );
      item.setTag( tag );
      item.setUrl( vm.get( "url").toString() );
      align::Helper alignHelper;
      item.setTextAlignment( alignHelper.findType( vm.get( "align").toString() ),
                             align::center );
    }
  }
}

void ListBox::setupUI(const vfs::Path & path)
{
  Widget::setupUI(path);
}

}//end namespace gui
