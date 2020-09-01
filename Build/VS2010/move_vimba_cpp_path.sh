#!/bin/bash

vimba_path='C:\Users\Public\Documents\Allied Vision\Vimba_4.0\VimbaCPP_Examples\'

source_str='..\..\..\..\Build\VS2010\'
echo $source_str
echo $vimba_path
# sed --debug -i  "s/'$source_str'/'$vimba_path'\Build\VS2010\/g" AsynchronousGrabMFC2.sln

awk '{sub(/\.\.\\\.\.\\\.\.\\\.\./,"C:\\Users\\Public\\Documents\\Allied Vision\\Vimba_4.0\\VimbaCPP_Examples")}1' AsynchronousGrabMFC3.vcxproj > temp1.vcxproj
cat temp1.vcxproj

# awk '{sub(/\.\.\\\.\.\\Source;\.\.\\\.\.\\\.\.\\\.\.\\\.\.;/, "\.\.\\\.\.\\Source;C:\\Users\\Public\\Documents\\Allied Vision\\Vimba_4.0\\VimbaCPP_Examples\\;")}1' temp1.vcxproj > temp2.vcxproj
# cat temp2.vcxproj
cp temp1.vcxproj AsynchronousGrabMFC.vcxproj


