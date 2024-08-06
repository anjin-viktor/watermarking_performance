eblind_dlc:
	Reference generation:
	eblind_dlc.exe --gen_reference --in=agriculture-hd.jpg --out=reference.bmp

	Embedding:
	eblind_dlc.exe --embed --in=agriculture-hd.jpg --reference=reference.bmp --out=test.png
	eblind_dlc.exe --embed --in=agriculture-hd.jpg --reference=reference.bmp --out=test_false.png --v=false

	Detection:
	eblind_dlc.exe --detect --in=agriculture-hd.jpg --reference=reference.bmp
