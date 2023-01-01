package com.github.awxkee.mozjpeg

import androidx.annotation.Keep

@Keep
class EncodeJpegException(override val message: String?): Exception(message) {
}