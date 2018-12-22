#pragma once

#include <memory>
#include <algorithm>
#include <map>
#include <cpp/ui/GraphicsUI.h>
#include <cpp/util/Utf16.h>
#include <cpp/Platform.h>

#pragma warning( push )
#pragma warning( disable: 4302 4838 )

#define NOMINMAX

#include <atlbase.h>
#include <atlwin.h>
#include <lib/wtl/atlapp.h>
#include <lib/wtl/atlcrack.h>
#include <lib/wtl/atlctrls.h>
#include <lib/wtl/atlmisc.h>
#include <lib/wtl/atlframe.h>
#include <lib/wtl/atldlgs.h>

#pragma warning( pop )


namespace cpp
{

    namespace windows
    {


        using Rect = cpp::GraphicsUI::Rect;
        using Widget = cpp::GraphicsUI::Widget;

        typedef cpp::Handle<CFont> Font;
        typedef cpp::Handle<CBrush> Brush;
        typedef cpp::Handle<CPen> Pen;


        class Convert
        {
        public:
            static Rect toRect(RECT r)
                { return Rect{ r.left, r.top, r.right - r.left, r.bottom - r.top }; }
            static RECT toRECT(Rect r)
                { return RECT{ (int)r.left(), (int)r.top(), (int)r.right(), (int)r.bottom() }; }
        };

        class ImageMap
        {
        public:
            ImageMap( );
            ~ImageMap( );

            void load( int resourceID );
            HBITMAP bitmap( int resourceID );

        private:
            std::map<int, HBITMAP> m_bitmaps;
        };


        class FontMap
        {
        public:
            FontMap( );

            void load( const cpp::Memory & id, int size, const cpp::Memory & fontName, bool bold = false, bool italic = false, bool underline = false, bool strikeout = false );
            Font & font( const cpp::Memory & id );

        private:
            std::map<cpp::String, Font> m_fonts;
        };


        class Graphics
        {
        public:
            static Graphics fromMemory()
                { return Graphics(); }

            Graphics( CDCHandle hdc )
                : m_hdc(hdc) { }
            Graphics( Graphics && move )
                : m_dc( ), m_hdc(move.m_hdc), m_font( std::move( move.m_font ) ) { m_dc = move.m_dc.Detach(); }
            Graphics( const Graphics & copy ) = delete;

            CDCHandle dc( )
                { return m_hdc; }

            void setFont( Font & font )
                { m_font = font; m_hdc.SelectFont( *font ); }

            void textOut( int x, int y, uint32_t foreColor, const wchar_t * text, size_t len );
            uint32_t textWidth( const cpp::Memory & text );
            uint32_t textWidth( const wchar_t * text, size_t len );

            void drawRectangle( Pen & outline, Brush & fill, int left, int top, int right, int bottom );
            void drawRectangle( Pen & outline, Brush & fill, RECT * rect );
            void drawRectangle( uint32_t color, const CRect & rect );

            static Font createFont( const cpp::Memory & fontName, int size )
                { Font font; font->CreateFontW( size, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CLEARTYPE_QUALITY, 0, cpp::toUtf16( fontName ).c_str()); return font; }
            static Brush createSolidBrush( uint32_t color )
                { return Brush{ CreateSolidBrush( toBGR( color ) ) }; }
            static Brush createSolidBrush( int a, int r, int g, int b )
                { return Brush{ CreateSolidBrush( toBGR(createColor(a,r,g,b) ) ) }; }
            static Brush stockBrush( int id )
                { return Brush{ (HBRUSH)GetStockObject( id ) }; }
            static Pen createPen( int style, int width, uint32_t color )
                { return Pen{ CreatePen(style, width, toBGR(color) ) }; }
            static uint32_t createColor( int a, int r, int g, int b )
                { return ( ( a & 0xff ) << 24 ) | ( ( r & 0xff ) << 16 ) | ( ( g & 0xff ) << 8 ) | ( ( b & 0xff ) << 0 ); }
            static uint32_t toBGR( uint32_t color )
                { return ( ( color & 0x00ff0000 ) >> 16 ) | ( color & 0x0000ff00 ) | ( ( color & 0x000000ff ) << 16 ); }
            static uint32_t sysColor( int index )
                { return toBGR( GetSysColor(index) ) | 0xff000000; }
        private:
            Graphics( )
                : m_dc( ), m_hdc() { m_dc.CreateCompatibleDC( nullptr ); m_hdc = m_dc; }

        private:
            CDC m_dc;
            CDCHandle m_hdc;
            Font m_font;
            uint32_t m_foreColor;
        };

        inline void Graphics::textOut( int x, int y, uint32_t foreColor, const wchar_t * text, size_t len )
        {
            if (m_foreColor != foreColor)
                { m_hdc.SetTextColor(toBGR(foreColor)); m_hdc.SetBkMode( TRANSPARENT ); }
            m_hdc.TextOut( x, y, text, (int)len );
        }

        inline uint32_t Graphics::textWidth( const wchar_t * text, size_t len )
        {
            SIZE size;
            m_hdc.GetTextExtent( text, (int)len, &size );
            return (uint32_t)size.cx;
        }

        inline uint32_t Graphics::textWidth( const cpp::Memory & text )
        {
            std::wstring wtext = toUtf16( text );
            return textWidth( wtext.c_str( ), wtext.length( ) );
        }

        inline void Graphics::drawRectangle( Pen & outline, Brush & fill, int left, int top, int right, int bottom )
        {
            m_hdc.SelectPen( *outline );
            m_hdc.SelectBrush( *fill );
    
            m_hdc.Rectangle( left, top, right + 1, bottom + 1 );
        }

        inline void Graphics::drawRectangle( Pen & outline, Brush & fill, RECT * rect )
        {
            drawRectangle( outline, fill, rect->left, rect->top, rect->right, rect->bottom );
        }

        inline void Graphics::drawRectangle( uint32_t color, const CRect & rect )
        {
            CDC tempHdc = CreateCompatibleDC( m_hdc );
            BLENDFUNCTION blend = {AC_SRC_OVER, 0, (color & 0xff000000) >> 24, 0};

            BITMAPINFO bmi = { 0 };        // bitmap header 

            CRect srcrect = { 0, 0, 4, 4 };

            // setup bitmap info  
            // set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on, the blending will occur in the center of each of the three areas. 
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = srcrect.Width();
            bmi.bmiHeader.biHeight = srcrect.Height();
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;         // four 8-bit components 
            bmi.bmiHeader.biCompression = BI_RGB;
            bmi.bmiHeader.biSizeImage = bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight * sizeof(uint32_t);

            // create our DIB section and select the bitmap into the dc 
            CBrush brush;
            brush.CreateSolidBrush( toBGR( color ) );

            CBitmap bitmap; bitmap.CreateDIBSection( tempHdc, &bmi, DIB_RGB_COLORS, NULL, NULL, 0x0 );
            tempHdc.SelectBitmap(bitmap);
            tempHdc.FillRect(&srcrect, brush);
            
            tempHdc.SetDCPenColor( toBGR( color ) );
            tempHdc.SetDCBrushColor( toBGR( color ) );

            m_hdc.AlphaBlend( rect.left, rect.top, rect.Width(), rect.Height(), tempHdc, srcrect.left, srcrect.top, srcrect.Width(), srcrect.Height(), blend );
        }

    }
}