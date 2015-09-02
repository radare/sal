TESTS=\
countdown.sal\
addition.sal\
retrun.sal\
swap.sal\
negate.sal\
float.sal\
regptr.sal\
arroba.sal\
arrobaptr.sal\
mul.sal\
net.sal\
shift.sal\
intend.sal\
system.sal\
array.sal\
nestloop.sal

%.sal : %.test $<
	@perl test.pl $<

all: ${TESTS}
