qcmdpc.py: qcmdpc.sage
	sage --preparse $<
	mv $<.py $@

.PHONY: check
check: ../../libtest.so
	sage -python main.py
