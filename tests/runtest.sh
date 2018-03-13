#! /bin/sh

cd tests || exit 1
for x in *.test; do
    result=`basename $x .test`.done
    if [ -r $result ]; then
	printf "%s: "  `basename $x .test`
	../imgfix < $x | diff - $result 2>/dev/null 1>&2 \
		&& echo ok \
		|| echo failed
    fi
done
exit 0
