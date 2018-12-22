#pragma once

#include <cpp/Platform.h>
#include <cpp/ui/ConsoleUI.h>

namespace cpp
{
    namespace windows
    {

        class ConsoleUI
            : public cpp::ConsoleUI
        {
        public:
            class Cursor
                : public cpp::ConsoleUI::Cursor
            {
            public:
                Cursor( HANDLE handle );

                virtual XY<int> position( ) const override;
                virtual void setPosition( XY<int> position ) override;

                virtual Color color( ) const override;
                virtual Color backColor( ) const override;
                virtual void setColor( Color color ) override;
                virtual void setBackColor( Color color ) override;

                virtual void hide( ) override;
                virtual void show( ) override;

            private:
                HANDLE m_handle;
                Color m_color;
                Color m_backColor;
                bool m_isHidden;
            };

        public:
            ConsoleUI( );

            virtual String title( ) const override;
            virtual void setTitle( const Memory & title ) override;

            virtual XY<int> bufferSize( ) const override;
            virtual void setBufferSize( XY<int> size ) override;

            virtual Rect<int> view( ) const override;
            virtual void setViewSize( XY<int> size ) override;
            virtual void setViewPosition( XY<int> size ) override;

            virtual Cursor & cursor( ) override;
            virtual void put( XY<int> pos, unicode_t ch, Color color = Color::White, Color backColor = Color::Black ) override;
            virtual void put( unicode_t ch, Color color = Color::White, Color backColor = Color::Black ) override;
            virtual void print( const Memory & string ) override;
            virtual void clear( Color backColor = Color::Black, Color color = Color::White, unicode_t ch = ' ' ) override;

            virtual unicode_t get( ) override;

        private:
            static int toTextAttr( Color color, Color backColor );

        private:
            HANDLE m_handle;
            CONSOLE_SCREEN_BUFFER_INFO m_bufferInfo;
            Cursor m_cursor;
        };
    }
}