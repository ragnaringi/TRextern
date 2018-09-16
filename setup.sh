#!/bin/sh

YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Download dependencies

readonly dependencyDir="`dirname "$0"`/dependencies" 

mkdir $dependencyDir
cd $dependencyDir
echo -e "${YELLOW}Downloading m_pd.h${NC}"
curl -O https://raw.githubusercontent.com/pure-data/pure-data/master/src/m_pd.h
echo -e "${YELLOW}Downloading Max SDK${NC}"
curl https://github.com/Cycling74/max-sdk/archive/v7.3.3.zip -L -o max-sdk-7.3.3.zip
echo -e "${YELLOW}Unzipping Max SDK${NC}"
unzip max-sdk-7.3.3.zip && rm max-sdk-7.3.3.zip
echo -e "${GREEN}All Done!${NC}"