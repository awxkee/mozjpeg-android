package com.radzivon.bartoshyk.mozjpeg

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.github.awxkee.mozjpeg.Mozjpeg
import com.radzivon.bartoshyk.mozjpeg.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val decodedBitmap = BitmapFactory.decodeResource(resources, R.drawable.test_png_with_alpha)
        val cc16 = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            decodedBitmap.copy(Bitmap.Config.RGBA_1010102, true)
        } else {
            decodedBitmap.copy(Bitmap.Config.ARGB_8888, true)
        }
        val encodedImage = Mozjpeg().compress(cc16, 81)
        val fixedBitmap = BitmapFactory.decodeByteArray(encodedImage, 0, encodedImage.size)
        binding.imageView.setImageBitmap(fixedBitmap)
    }
}