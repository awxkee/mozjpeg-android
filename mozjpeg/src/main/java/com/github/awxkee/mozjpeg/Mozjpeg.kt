package com.github.awxkee.mozjpeg

import android.graphics.Bitmap

class Mozjpeg {

    fun compress(bitmap: Bitmap, @androidx.annotation.IntRange(from = 0, to = 100) quality: Int): ByteArray {
        return compressImpl(bitmap, quality)
    }

    private external fun compressImpl(
        bitmap: Bitmap,
        quality: Int
    ): ByteArray

    companion object {
        // Used to load the 'mozjpeg' library on application startup.
        init {
            System.loadLibrary("mozjpeg")
        }
    }
}