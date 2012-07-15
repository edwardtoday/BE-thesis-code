mkdir L
ffmpeg -i twoeye_720x576_L.mp4 L/twoeye_720x576%04d.jpg
mkdir R
ffmpeg -i twoeye_720x576_R.mp4 R/twoeye_720x576%04d.jpg
