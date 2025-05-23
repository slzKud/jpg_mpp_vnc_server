# jpg_mpp_vnc_server
使用Rockchip MPP和RGA硬件加速JPEG解码的[vnc_mjpeg_server](https://github.com/slzKud/vnc_mjpeg_server)修改版。

在搭载Rockchip RK3588的[香橙派5 Plus](http://www.orangepi.cn/orangepiwiki/index.php/Orange_Pi_5_Plus)上测试通过。

## 编译安装操作说明

编译和依赖和[vnc_mjpeg_server](https://github.com/slzKud/vnc_mjpeg_server)基本一致。

1. 下载瑞芯微官方的[MPP](https://github.com/rockchip-linux/mpp)和[RGA](https://github.com/airockchip/librga)。
2. 将MPP和RGA源码放在和jpg_mpp_vnc_server源码同级的目录，并正常编译类库。
3. 后续按照vnc_mjpeg_server项目Readme编译即可。

如果编译过程报错，请更改CMakeList的MPP和RGA库路径。因为MPP和RGA的API经常发生变化。可能后续变化了也需要更改API。（2025.3的的版本应该是没问题的）

## 运行说明
编译后会生成两个文件```jpeg-server```和```mjpeg-server```。

```jpeg-server```是测试用的静态JPG服务器，以```./jpeg-server [jpg_path]```运行即可。

```mjpeg-server```是[vnc_mjpeg_server](https://github.com/slzKud/vnc_mjpeg_server)的修改版本，参照该项目Readme运行即可。

## 已知问题
* 因为neatvnc的兼容性问题，使用tight编码会发生随机崩溃。可以尝试更改neatVNC代码缓解，但是无法避免。建议测试使用ZRLE或者RAW编码。

* 因为修改侧重点是给解码加上MPP硬解码支持，对framebuffer的代码没有做太多优化，所以CPU占用还是相对较高。（使用Bad Apple演示视频,占用大约在单核30-50%。1080p演示适配在大约单核90%左右）