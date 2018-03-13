#! /bin/sh

cd tests || exit 1
for x in *.test; do
    result=`basename $x .test`.done
    if [ -r $result ]; then
	cp $x /tmp/$$.test
	printf "%s: "  `basename $x .test`
	if ../imgfix /tmp/$$.test; then
	    diff /tmp/$$.test  $result 2>/dev/null 1>&2 \
		&& echo ok \
		|| echo failed
	else
	    echo failed
	fi
	rm /tmp/$$.test
    fi
done
exit 0
