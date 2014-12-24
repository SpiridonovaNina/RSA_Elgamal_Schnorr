all:
	cd bigNumber && make

clear:
	rm *.txt
	rm *.pyc
	rm *.priv
	rm *.pub
	cd bigNumber && make clear