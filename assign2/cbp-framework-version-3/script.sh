#!/bin/bash 
touch testdata.log 
echo "-------------------------------------------------------------------------------------------------------" >> testdata.log
echo "INT1" >> testdata.log 
./predictor traces/without-values/DIST-INT-1 >> testdata.log 
echo "INT2" >> testdata.log 
./predictor traces/without-values/DIST-INT-2 >> testdata.log 
echo "INT3" >> testdata.log 
./predictor traces/without-values/DIST-INT-3 >> testdata.log 
echo "MM1" >> testdata.log 
./predictor traces/without-values/DIST-MM-1 >> testdata.log 
echo "MM2" >> testdata.log 
./predictor traces/without-values/DIST-MM-2 >> testdata.log 
echo "SERV1" >> testdata.log 
./predictor traces/without-values/DIST-SERV-1 >> testdata.log 
echo "SERV2" >> testdata.log 
./predictor traces/without-values/DIST-SERV-2 >> testdata.log 
echo "SERV3" >> testdata.log 
./predictor traces/without-values/DIST-SERV-3 >> testdata.log 
echo "SERV4" >> testdata.log 
./predictor traces/without-values/DIST-SERV-4 >> testdata.log 
echo "SERV5" >> testdata.log 
./predictor traces/without-values/DIST-SERV-5 >> testdata.log
echo "-------------------------------------------------------------------------------------------------------" >> testdata.log
