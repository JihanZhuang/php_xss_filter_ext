PHP_ARG_ENABLE(sec,
      [Whether to enable the "sec" extension],
      [  --enable-sec      Enable "sec" extension support])

if test $PHP_SEC != "no"; then
     PHP_NEW_EXTENSION(sec, sec.c, $ext_shared)
     PHP_SUBST(SEC_SHARED_LIBADD)
fi
