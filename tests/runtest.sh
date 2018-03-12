#! /bin/sh

cd tests || exit 1
for x in *.test; do
    result=`basename $x .test`.done
    printf "%s: "  `basename $x .test`; ../imgfix < $x | diff - $result && echo ok || echo failed
done
exit 0
