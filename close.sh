#!/bin/bash
lsof -i :5000 && 
kill -9 $(lsof -t -i :5000)