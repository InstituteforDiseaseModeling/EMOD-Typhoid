
echo Setting PATH=Z:\;Z:\Python27;%PATH%
set PATH=Z:\;c:\Python27;%PATH%

:: 'Official' Python path environment variable
set PYTHONPATH=c:\Python27

:: IDM Python path environment variable
set PYTHON_PATH=c:\Python27

:: https://docs.python.org/2/using/windows.html
set PYTHONHOME=c:\Python27

Z:\Eradication.exe -C config.xlsm -O testing -I Z:\Scenarios\InputFiles --python-script-path=z:\Python
