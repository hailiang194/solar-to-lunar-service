# Solar to lunar service

Convert Solar calendar(Gregorian calendar) to Lunar calendar(陰曆) based on the idea of [Hồ Ngọc Đức](https://www.informatik.uni-leipzig.de/~duc/amlich/)

## Installation

Run Dockerfile

## API Guide

|Method|Path|Descprtion|
|------|----|----------|
|```GET```|```api/day?query=dd/MM/yyyy&<tz=7.0>```|Convert query date to lunar date, default timezones is GMT+7.0(VN)|