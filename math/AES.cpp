#include <algorithm>
#include <cpp/math/AES.h>
#include <cpp/math/rijndael.h>

namespace cpp
{

    class Aes256
    {
    public:
        void initEncrypt( String cipherKey, const Memory & initVector );
        String encrypt( const Memory & plainText, bool padding = false );
    public:
        void initDecrypt( String cipherKey, const Memory & initVector );
        String decrypt( const Memory & cipherText, bool padding = false );
    public:
        static String encrypt( const Memory & plainText, const Memory & cipherKey, const Memory & initVector );
        static String decrypt( const Memory & cipherText, const Memory & cipherKey, const Memory & initVector );
    private:
        static const int kKeyLen = 256;
        static const int kBlockSize = 16;
        static const int kKeyNR = 14;
        static const int kMAXNR = 14;
        uint32_t rk[4 * ( kMAXNR + 1 )];       //    key schedule
        uint32_t iv[4];
    };

    void Aes256::initEncrypt( String cipherKey, const Memory & initVector )
    {
        cipherKey.resize( 32 );
        rijndael::rijndaelKeySetupEnc( rk, (uint8_t *)cipherKey.c_str( ), kKeyLen );
        memcpy( iv, initVector.data( ), std::min( initVector.length( ), (size_t)kBlockSize ) );
        if ( initVector.length( ) < kBlockSize )
        {
            memset( iv + initVector.length( ), 0, kBlockSize - initVector.length( ) );
        }
    }

    String Aes256::encrypt( const Memory & plainText, bool padding )
    {
        uint8_t block[kBlockSize];
        int numBlocks = (int)plainText.length( ) / kBlockSize + ( padding ? 1 : 0 );
        String cipherText( numBlocks * kBlockSize, 0 );

        uint8_t * input = (uint8_t *)plainText.data( );
        uint8_t * output = (uint8_t *)cipherText.c_str( );

        for ( int i = 0; i<numBlocks; i++ )
        {
            ( (uint32_t*)block )[0] = ( (uint32_t*)input )[0] ^ iv[0];
            ( (uint32_t*)block )[1] = ( (uint32_t*)input )[1] ^ iv[1];
            ( (uint32_t*)block )[2] = ( (uint32_t*)input )[2] ^ iv[2];
            ( (uint32_t*)block )[3] = ( (uint32_t*)input )[3] ^ iv[3];
            rijndael::rijndaelEncrypt( rk, kKeyNR, block, output );
            memcpy( iv, output, kBlockSize );
            input += kBlockSize;
            output += kBlockSize;
        }
        if ( padding )
        {
            int padLen = kBlockSize - ( cipherText.length( ) % kBlockSize );
            for ( int i = 0; i<kBlockSize; i++ )
            {
                block[i] = ( i<padLen ? input[i] : (uint8_t)padLen ) ^ ( (uint8_t *)iv )[i];
            }
            rijndael::rijndaelEncrypt( rk, kKeyNR, block, output );
            memcpy( iv, output, kBlockSize );
        }
        return cipherText;
    }

    void Aes256::initDecrypt( String cipherKey, const Memory & initVector )
    {
        cipherKey.resize( 32 );
        rijndael::rijndaelKeySetupDec( rk, (uint8_t *)cipherKey.c_str( ), kKeyLen );
        memcpy( iv, initVector.data( ), std::min( initVector.length( ), (size_t)kBlockSize ) );
        if ( initVector.length( ) < kBlockSize )
        {
            memset( iv + initVector.length( ), 0, kBlockSize - initVector.length( ) );
        }
    }

    String Aes256::decrypt( const Memory & cipherText, bool padding )
    {
        uint8_t block[kBlockSize];
        int numBlocks = (int)cipherText.length( ) / kBlockSize - ( padding ? 1 : 0 );
        String plainText( cipherText.length( ), 0 );

        uint8_t * input = (uint8_t *)cipherText.data( );
        uint8_t * output = (uint8_t *)plainText.c_str( );

        /* all blocks but last */
        for ( int i = 0; i<numBlocks; i++ )
        {
            rijndael::rijndaelDecrypt( rk, kKeyNR, input, block );
            ( (uint32_t*)block )[0] ^= iv[0];
            ( (uint32_t*)block )[1] ^= iv[1];
            ( (uint32_t*)block )[2] ^= iv[2];
            ( (uint32_t*)block )[3] ^= iv[3];
            memcpy( iv, input, kBlockSize );
            memcpy( output, block, kBlockSize );
            input += kBlockSize;
            output += kBlockSize;
        }
        /* last block */
        if ( padding )
        {
            rijndael::rijndaelDecrypt( rk, kKeyNR, input, block );
            ( (uint32_t*)block )[0] ^= iv[0];
            ( (uint32_t*)block )[1] ^= iv[1];
            ( (uint32_t*)block )[2] ^= iv[2];
            ( (uint32_t*)block )[3] ^= iv[3];
            int padLen = block[kBlockSize - 1];
            if ( padLen <= 0 || padLen>kBlockSize )
            {
                return plainText;
            }
            for ( int i = kBlockSize - padLen; i<kBlockSize; i++ )
            {
                if ( block[i] != padLen )
                {
                    return plainText;
                }
            }
            memcpy( output, block, kBlockSize - padLen );
        }
        return plainText;
    }



    namespace AES
    {

        String encrypt256( const Memory & plainText, const Memory & cipherKey, const Memory & initVector )
        {
            Aes256 aes;
            aes.initEncrypt( cipherKey, initVector );
            return aes.encrypt( plainText );
        }

        String decrypt256( const Memory & cipherText, const Memory & cipherKey, const Memory & initVector )
        {
            Aes256 aes;
            aes.initDecrypt( cipherKey, initVector );
            return aes.decrypt( cipherText );
        }

    }

}