# CS621 - Compression Link Simulator
## How to Run
- have ns3 installed
- cd into directory with waf
```
./waf --run "scratch/cda --capacity=<capacity> --compressionEnabled=<0 or 1>"
```
- <capacity> is the capacity of the compression link in Mbps
- <compressionEnabled> is whether the compression is enabled in the link or not

### Add ons
- Compression algorithms provided by [zlib](https://zlib.net/)
- Json parsing from [JsonCPP](https://github.com/open-source-parsers/jsoncpp)
