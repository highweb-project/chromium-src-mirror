하이웹 브라우저 빌드 가이드
===========================

소개
----

하이웹 브라우저는 웹 고속화 관련 수정 사항이 적용된 크로미엄 61 기반의 브라우저 입니다.

-	**WebCL**
	-	WebCL 1.0 (Draft) 규격을 지원합니다.
		-	https://www.khronos.org/webcl/
	-	WebCL은 자바스크립트를 통한 OpenCL 병렬 연산을 지원합니다.
	-	하이웹 브라우저로 아래 주소의 WebCL 샘플들을 실행 할 수 있습니다.
		-	https://highweb-project.github.io/WebCL-sample/
	- 오류 사항을 수정한 크로노스 WebCL Conformance Test를 아래 링크에서 확인할 수 있습니다.
		- https://highweb-project.github.io/WebCL-conformance/webcl-conformance-tests.html

-	**WebVKc**
	-	WebVKc (가칭 WebVulkan Compute, 실험적인 기능) 규격을 지원합니다.
	-	Vulkan의 Compute Shader 기능을 자바스크립트에서 사용할 수 있도록 API를 제공합니다.
	-	Khronos Vulkan
		-	https://www.khronos.org/vulkan/
	-	WebVKc IDL
		-	https://highweb-project.github.io/webvkc-specification/
	-	WebVKc 샘플 (기존 WebCL 샘플 컨버팅)
		-	https://highweb-project.github.io/WebVKc-sample/

-	**DeviceAPI 확장**
	-	자바스크립트 상에서 단말 자원에 접근할 수 있도록 자바스크립트 확장을 제공합니다.
		-	ApplicationLauncher, SystemInformation, Calendar, Contact, Messaging, Gallery, Sensor
	-	각 기능들에 대한 동작 및 샘플 코드는 아래 링크를 참고해 주세요.
		-	https://highweb-project.github.io/DeviceAPI-sample/

- **SVG-Canvas 변환 및 벤치마크**
	- 특정 SVG 태그를 캔버스 변환 스크립트를 사용해서 캔버스로 그리도록 수정합니다.
	- 위 작업을 통해 페인팅 작업의 속도 향상이 가능한지 개발자 도구를 이용해서 측정합니다.
<pre><code>./out/LinuxRelease/chrome -auto-open-devtools-for-tabs http://211.45.65.49/svgcanvas-benchmark/svgbenchmark_v4.php
</code></pre>

-	**HTML5 추가 기능 활성화**
	-	크로미엄 내부에 구현된 실험적인 기능들을 테스트 할 수 있도록 관련 기능들을 활성화 합니다.

크로미엄 브라우저 빌드
----------------------

빌드 절차는 크로미엄 브라우저와 동일합니다. 상세한 크로미엄 브라우저 빌드 가이드는 아래 링크를 참고해 주세요.
이 문서에서는 단순화된 빌드 절차를 명령어와 함께 안내합니다.

https://www.chromium.org/developers/how-tos/get-the-code

빌드에 필요한 사양
------------------

-	우분투 16.04 64비트, 램 8기가 이상, 가급적이면 SSD 권장
-	depot_tools 사전 설정
	-	fetch, gclient 명령어 사용을 위해 사전에 depot_tools가 설정되어 있어야 합니다.
	자세한 설정 방법은 아래 링크를 참고하세요.
		-	https://dev.chromium.org/developers/how-tos/install-depot-tools

소스 다운로드
---------------

-	하이웹 브라우저 깃허브 저장소를 기준으로 하는 gclient 환경파일 생성
<pre><code>highweb-browser$ gclient config --name=src https://github.com/highweb-project/highweb-browser.git@origin/20170926 --unmanaged --deps-file=.DEPS.git</code></pre>

-	안드로이드 관련 소스 파일을 추가 다운로드 받도록 옵션 추가
<pre><code>highweb-browser$ echo "target_os = [\"android\"]" >> .gclient</code></pre>

-	외부 소스 저장소 동기화
<pre><code>highweb-browser$ gclient sync -n --with_branch_heads</code></pre>

-	필요 라이브러리 설치 (최초 1회만 필요)
<pre><code>src$ ./build/install-build-deps-android.sh
src$ ./build/install-build-deps.sh
src$ ./build/install-build-deps.sh --arm
src$ ./build/linux/sysroot_scripts/install-sysroot.py --arch=arm
src$ gclient runhooks
</code></pre>

CCACHE 설치
---------------
- 빌드에 ccache를 사용합니다.
<pre><code>$ sudo apt install ccache
</code></pre>
- ccache 사용을 위한 환경 값을 설정합니다.
<pre><code>$ export CCACHE_CPP2=yes
$ export CCACHE_SLOPPINESS=time_macros
</code></pre>

GN 빌드
---------------
- 리눅스, 안드로이드, 오드로이드 빌드를 위한 빌드 환경 생성
<pre><code>gn gen out/LinuxRelease --args='enable_highweb_svgconvert=true enable_highweb_deviceapi=true enable_highweb_webvkc=true enable_highweb_webcl=true is_debug=false symbol_level=0 proprietary_codecs=true ffmpeg_branding="Chrome" cc_wrapper="ccache" enable_nacl=false remove_webcore_debug_symbols=true linux_use_bundled_binutils=false use_debug_fission=false clang_use_chrome_plugins=false use_jumbo_build=true'
gn gen out/LinuxDebug --args='enable_highweb_svgconvert=true enable_highweb_deviceapi=true enable_highweb_webvkc=true enable_highweb_webcl=true is_debug=true symbol_level=1 proprietary_codecs=true ffmpeg_branding="Chrome" cc_wrapper="ccache" enable_nacl=false remove_webcore_debug_symbols=true linux_use_bundled_binutils=false use_debug_fission=false clang_use_chrome_plugins=false use_jumbo_build=true'
gn gen out/AndroidRelease --args='enable_highweb_svgconvert=true enable_highweb_deviceapi=true enable_highweb_webvkc=true enable_highweb_webcl=true is_debug=false symbol_level=0 proprietary_codecs=true ffmpeg_branding="Chrome" cc_wrapper="ccache" enable_nacl=false remove_webcore_debug_symbols=true linux_use_bundled_binutils=false use_debug_fission=false clang_use_chrome_plugins=false use_jumbo_build=true target_os="android"'
gn gen out/AndroidReleaseX86 --args='enable_highweb_svgconvert=true enable_highweb_deviceapi=true enable_highweb_webvkc=true enable_highweb_webcl=true is_debug=false symbol_level=0 proprietary_codecs=true ffmpeg_branding="Chrome" cc_wrapper="ccache" enable_nacl=false remove_webcore_debug_symbols=true linux_use_bundled_binutils=false use_debug_fission=false clang_use_chrome_plugins=false use_jumbo_build=true target_os="android" target_cpu="x86"'
gn gen out/AndroidDebug --args='enable_highweb_svgconvert=true enable_highweb_deviceapi=true enable_highweb_webvkc=true enable_highweb_webcl=true is_debug=true symbol_level=1 proprietary_codecs=true ffmpeg_branding="Chrome" cc_wrapper="ccache" enable_nacl=false remove_webcore_debug_symbols=true linux_use_bundled_binutils=false use_debug_fission=false clang_use_chrome_plugins=false use_jumbo_build=true target_os="android"'
gn gen out/OdroidRelease --args='enable_highweb_svgconvert=true enable_highweb_deviceapi=true enable_highweb_webvkc=true enable_highweb_webcl=true is_debug=false symbol_level=0 proprietary_codecs=true ffmpeg_branding="Chrome" cc_wrapper="ccache" enable_nacl=false remove_webcore_debug_symbols=true linux_use_bundled_binutils=false use_debug_fission=false clang_use_chrome_plugins=false use_jumbo_build=true target_cpu="arm" use_allocator="none"'
</code></pre>

안드로이드 빌드
---------------
-	ninja 빌드 진행
<pre><code>src$ ninja -C out/AndroidRelease chrome_public_apk
</code></pre>

-	APK 설치 및 실행
<pre><code>src$ adb uninstall org.chromium.chrome; adb install -r ./out/AndroidRelease/apks/ChromePublic.apk";
src$ adb shell am force-stop org.chromium.chrome; adb shell am start -a android.intent.action.MAIN -n org.chromium.chrome/com.google.android.apps.chrome.Main";
</code></pre>

리눅스 (x64) 빌드
-----------------
-	ninja 빌드 진행
<pre><code>src$ ninja -C out/LinuxRelease chrome
</code></pre>

-	크로미엄 실행  
<pre><code>src$ ./out/LinuxRelease/chrome
</code></pre>

오드로이드 리눅스 (x86, arm) 빌드
---------------------------------
-	ninja 빌드 진행
<pre><code>src$ ninja -C out/OdroidRelease chrome
</code></pre>

- 크로미엄 실행
	- 오드로이드로 OdroidRelease 디렉토리 전송 후 실행
<pre><code>$ unset GTK_IM_MODULE; ./chrome --use-gl=egl
</code></pre>

-	libgcrypt11 관련 오류 발생 시
	-	우분투 16.04 MATE (20161011) 버전에는 libgcrypt11 라이브러리를 설치해야 합니다.
<pre><code>$ wget https://launchpad.net/ubuntu/+archive/primary/+files/libgcrypt11_1.5.3-2ubuntu4.2_armhf.deb
$ sudo dpkg -i libgcrypt11_1.5.3-2ubuntu4.2_armhf.deb
</code></pre>

개발 관련 팁
-----------------
- icecc 분산 빌드 적용
	- 아래 환경 값을 설정 후 동일하게 빌드 진행합니다.
<pre><code>src$ export CCACHE_PREFIX=icecc
src$ export ICECC_CLANG_REMOTE_CPP=1
src$ export ICECC_VERSION=`pwd`/clang.tar.gz
src$ export PATH=$PATH:`pwd`/third_party/llvm-build/Release+Asserts/bin
</code></pre>
- 안드로이드 하이웹 브라우저가 죽을 경우 스택 확인 (디버그 빌드 사용)
<pre><code>src$ cd out/AndroidDebug; adb logcat -d | ../../third_party/android_platform/development/scripts/stack; cd ../..
</code></pre>
- 크로미엄 캐시 및 환경 값 삭제
<pre><code>$ rm -rf ~/.config/chromium/
$ rm -rf ~/.cache/chromium/
</code></pre>
