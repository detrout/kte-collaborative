// This file is generated by kconfig_compiler from kobbysettings.kcfg.
// All changes you do to this file will be lost.
#ifndef KOBBY_KOBBYSETTINGS_H
#define KOBBY_KOBBYSETTINGS_H

#include <kconfigskeleton.h>
#include <kdebug.h>

namespace Kobby {

class KobbySettings : public KConfigSkeleton
{
  public:

    static KobbySettings *self();
    ~KobbySettings();

    /**
      Set Width of the Main Window.
    */
    static
    void setMainWindowWidth( int v )
    {
      if (!self()->isImmutable( QString::fromLatin1 ( "MainWindowWidth" ) ))
        self()->mMainWindowWidth = v;
    }

    /**
      Get Width of the Main Window.
    */
    static
    int mainWindowWidth()
    {
      return self()->mMainWindowWidth;
    }

    /**
      Set Height of the Main Window.
    */
    static
    void setMainWindowHeight( int v )
    {
      if (!self()->isImmutable( QString::fromLatin1 ( "MainWindowHeight" ) ))
        self()->mMainWindowHeight = v;
    }

    /**
      Get Height of the Main Window.
    */
    static
    int mainWindowHeight()
    {
      return self()->mMainWindowHeight;
    }

    /**
      Set X location of Main Window.
    */
    static
    void setMainWindowX( int v )
    {
      if (!self()->isImmutable( QString::fromLatin1 ( "MainWindowX" ) ))
        self()->mMainWindowX = v;
    }

    /**
      Get X location of Main Window.
    */
    static
    int mainWindowX()
    {
      return self()->mMainWindowX;
    }

    /**
      Set Y location of Main Window.
    */
    static
    void setMainWindowY( int v )
    {
      if (!self()->isImmutable( QString::fromLatin1 ( "MainWindowY" ) ))
        self()->mMainWindowY = v;
    }

    /**
      Get Y location of Main Window.
    */
    static
    int mainWindowY()
    {
      return self()->mMainWindowY;
    }

    /**
      Set Sizes of main horizontal splitter.
    */
    static
    void setMainWindowHorizSplitterSizes( const QList<int> & v )
    {
      if (!self()->isImmutable( QString::fromLatin1 ( "MainWindowHorizSplitterSizes" ) ))
        self()->mMainWindowHorizSplitterSizes = v;
    }

    /**
      Get Sizes of main horizontal splitter.
    */
    static
    QList<int> mainWindowHorizSplitterSizes()
    {
      return self()->mMainWindowHorizSplitterSizes;
    }

    /**
      Set Nick Name other users see.
    */
    static
    void setNickName( const QString & v )
    {
      if (!self()->isImmutable( QString::fromLatin1 ( "NickName" ) ))
        self()->mNickName = v;
    }

    /**
      Get Nick Name other users see.
    */
    static
    QString nickName()
    {
      return self()->mNickName;
    }

    /**
      Set Color user appears as on server.
    */
    static
    void setUserColor( const QColor & v )
    {
      if (!self()->isImmutable( QString::fromLatin1 ( "UserColor" ) ))
        self()->mUserColor = v;
    }

    /**
      Get Color user appears as on server.
    */
    static
    QColor userColor()
    {
      return self()->mUserColor;
    }

    /**
      Set Hostname of local computer.
    */
    static
    void setHostName( const QString & v )
    {
      if (!self()->isImmutable( QString::fromLatin1 ( "HostName" ) ))
        self()->mHostName = v;
    }

    /**
      Get Hostname of local computer.
    */
    static
    QString hostName()
    {
      return self()->mHostName;
    }

  protected:
    KobbySettings();
    friend class KobbySettingsHelper;


    // ui
    int mMainWindowWidth;
    int mMainWindowHeight;
    int mMainWindowX;
    int mMainWindowY;
    QList<int> mMainWindowHorizSplitterSizes;

    // profile
    QString mNickName;
    QColor mUserColor;

    // network
    QString mHostName;

  private:
};

}

#endif

