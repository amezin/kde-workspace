//////////////////////////////////////////////////////////////////////////////
// nitrogen.cpp
// -------------------
// 
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
// Copyright (c) 2006, 2007 Riccardo Iaconelli <riccardo@kde.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.                 
//////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <kdeversion.h>
#include <kwindowinfo.h>
#include <QApplication>
#include <QPainter>

#include "nitrogen.h"
#include "nitrogen.moc"
#include "nitrogenclient.h"


extern "C"
{
  KDE_EXPORT KDecorationFactory* create_factory()
  { return new Nitrogen::NitrogenFactory(); }
}

namespace Nitrogen
{
  
  // referenced from definition in Nitrogendclient.cpp
  OxygenHelper *nitrogenHelper(); 
  
  // initialize static members
  bool NitrogenFactory::initialized_ = false;
  NitrogenConfiguration NitrogenFactory::defaultConfiguration_;
  NitrogenExceptionList NitrogenFactory::exceptions_;
  
  //___________________________________________________
  NitrogenFactory::NitrogenFactory()
  {
    readConfig();
    setInitialized( true );
  }
  
  //___________________________________________________
  NitrogenFactory::~NitrogenFactory() 
  { setInitialized( false ); }
  
  //___________________________________________________
  KDecoration* NitrogenFactory::createDecoration(KDecorationBridge* bridge )
  { 
    NitrogenClient* client( new NitrogenClient( bridge, this ) );
    connect( this, SIGNAL( configurationChanged() ), client, SLOT( resetConfiguration() ) );
    return client->decoration(); 
  }
  
  //___________________________________________________
  bool NitrogenFactory::reset(unsigned long changed)
  {
    
    kDebug( 1212 ) << endl;
    
    // read in the configuration
    setInitialized( false );
    bool configuration_changed = readConfig();
    setInitialized( true );
    
    if( configuration_changed || (changed & (SettingDecoration | SettingButtons | SettingBorder)) ) 
    { 
      
      emit configurationChanged();
      return true; 
      
    } else {
      
      emit configurationChanged();
      resetDecorations(changed);
      return false;
      
    }
    
  }
  
  //___________________________________________________
  bool NitrogenFactory::readConfig()
  {

    kDebug( 1212 ) << endl;
    
    // create a config object
    KConfig config("nitrogenrc");
    KConfigGroup group( config.group("Windeco") );
    NitrogenConfiguration configuration( group );
    bool changed = !( configuration == defaultConfiguration() );

    // read exceptionsreadConfig
    NitrogenExceptionList exceptions( config );
    if( !( exceptions == exceptions_ ) ) 
    {
      exceptions_ = exceptions; 
      changed = true;
    }
    
    
    if( changed ) 
    {
      
      nitrogenHelper()->invalidateCaches();
      setDefaultConfiguration( configuration );
      return true;
      
    } else return false;
    
  }
  
  //_________________________________________________________________
  bool NitrogenFactory::supports( Ability ability ) const
  {
    switch( ability ) 
    {
      
      // announce
      case AbilityAnnounceButtons:
      case AbilityAnnounceColors:
      
      // buttons
      case AbilityButtonMenu:
      case AbilityButtonHelp:
      case AbilityButtonMinimize:
      case AbilityButtonMaximize:
      case AbilityButtonClose:
      case AbilityButtonOnAllDesktops:
      case AbilityButtonAboveOthers:
      case AbilityButtonBelowOthers:
      case AbilityButtonSpacer:
      case AbilityButtonShade:
      
      //       // colors
      //       case AbilityColorTitleBack:
      //       case AbilityColorTitleFore:
      //       case AbilityColorFrame:
      
      // compositing
      case AbilityProvidesShadow: // TODO: UI option to use default shadows instead
      case AbilityUsesAlphaChannel:
      return true;
            
      // no colors supported at this time
      default:
      return false;
    };
  }
  
  
  
  //____________________________________________________________________
  NitrogenConfiguration NitrogenFactory::configuration( const NitrogenClient& client )
  {
    
    QString window_title;
    QString class_name;
    for( NitrogenExceptionList::const_iterator iter = exceptions_.constBegin(); iter != exceptions_.constEnd(); iter++ )
    {
      
      // discard disabled exceptions
      if( !iter->enabled() ) continue;
      
      /* 
      decide which value is to be compared
      to the regular expression, based on exception type
      */
      QString value;
      switch( iter->type() )
      {
        case NitrogenException::WindowTitle: 
        {
          value = window_title.isEmpty() ? (window_title = client.caption()):window_title;
          break;
        }
        
        case NitrogenException::WindowClassName:
        {
          if( class_name.isEmpty() )
          {
            // retrieve class name
            KWindowInfo info( client.windowId(), 0, NET::WM2WindowClass );
            QString window_class_name( info.windowClassName() );
            QString window_class( info.windowClassClass() );
            class_name = window_class_name + " " + window_class;
          } 
          
          value = class_name;
          break;
        }
        
        default: assert( false );
        
      }
      
      if( iter->regExp().indexIn( value ) < 0 ) continue;
      
      
      NitrogenConfiguration configuration( defaultConfiguration() );
      
      // propagate all features found in mask to the output configuration
      if( iter->mask() & NitrogenException::FrameBorder ) configuration.setFrameBorder( iter->frameBorder() );
      if( iter->mask() & NitrogenException::BlendColor ) configuration.setBlendColor( iter->blendColor() );
      if( iter->mask() & NitrogenException::DrawSeparator ) configuration.setDrawSeparator( iter->drawSeparator() );
      if( iter->mask() & NitrogenException::TitleOutline ) configuration.setDrawTitleOutline( iter->drawTitleOutline() );
      if( iter->mask() & NitrogenException::SizeGripMode ) configuration.setSizeGripMode( iter->sizeGripMode() );
      
      return configuration;
      
    }
    
    return defaultConfiguration();
    
  }
  
} //namespace Nitrogen
