@echo off


SET PATH_ROOT=%~dp0


cd "%PATH_ROOT%"

IF exist http-parser (
    cd http-parser
    git pull --ff-only

) ELSE (
    git clone https://github.com/nodejs/http-parser.git
)


cd "%PATH_ROOT%"

IF exist qhttp (
    cd qhttp
    git pull --ff-only

) ELSE (
    git clone https://github.com/azadkuh/qhttp.git
)
