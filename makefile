python_shell="import sys; print \"makefile.vc\" if sys.platform.startswith(\"win\") else \"makefile.linux\""
makefile=$(shell python -c $(python_shell))
include $(makefile)
