
#!/bin/bash
workdir=../../build-deepin-deb-installer-unknown-Debug/test #编译路径
executable=deepin-deb-installer-test #可执行程序的文件名
build_dir=$workdir
cd $build_dir
result_coverage_dir=coverage
result_report_dir=`pwd`/report/report.xml
#下面是覆盖率目录操作，一种正向操作，一种逆向操作
extract_info="*/application/*"  #针对当前目录进行覆盖率操作
./$executable --gtest_output=xml:$result_report_dir

lcov --directory `pwd` --capture --output-file coverage.info
#    #3.过滤一些我们不感兴趣的文件的覆盖率信息
lcov --remove coverage.info '*/deepin-deb-installer-test_autogen/*' '*/deepin-deb-installer-test_autogen/*' '*/usr/include/*' '*/dbuslogin1manager*' '*/tests/*' '*/googletest/*' '*/3rdparty/*' '*/test/*' '*/accessible.h' '*/accessibledefine.h' -o coverage.info
genhtml -o $result_coverage_dir coverage.info
nohup x-www-browser `pwd`/$result_coverage_dir/index.html &
nohup x-www-browser $result_report_dir &
 
lcov -d `pwd` –z
exit 0
