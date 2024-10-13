#!/bin/bash
netstat -an | grep :5000 &&
lsof -i :5000 && 
kill -9 $(lsof -t -i :5000)