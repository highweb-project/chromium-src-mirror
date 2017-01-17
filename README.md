하이웹 브라우저 빌드 가이드
===========================

소개
----

하이웹 브라우저는 크로미엄 오픈소스 브라우저에 웹고속화 관련 수정 사항들이 적용된 브라우저입니다. 현재 아래 내용들이 적용되어 있으며 기반 브라우저는 크로미엄 57.0.2951.0 버전입니다.

-	**WebCL**
	-	WebCL 1.0 (Draft) 규격을 지원합니다.
		-	https://www.khronos.org/webcl/
	-	WebCL은 브라우저에서 자바스크립트를 통해 OpenCL API를 이용 병렬 연산이 가능하도록 도와줍니다.
	-	하이웹 브라우저로 아래 주소의 WebCL 샘플들을 실행 확인할 수 있습니다.
		-	https://highweb-project.github.io/WebCL-sample/
	- 오류 사항을 수정한 크로노스 WebCL Conformance Test를 아래 링크에서 확인할 수 있습니다.
		- https://highweb-project.github.io/WebCL-conformance/webcl-conformance-tests.html

-	**WebVKc**
	-	WebVKc (가칭 WebVulkan Compute, 실험적인 기능) 규격을 지원합니다.
	-	Vulkan의 Compute Shader 기능을 자바스크립트에서 사용할 수 있도록 관련 API를 제공합니다.
	-	Khronos Vulkan
		-	https://www.khronos.org/vulkan/
	-	WebVKc IDL
		-	(준비중)
	-	WebVKc 샘플 (기존 WebCL 샘플 컨버팅)
		-	https://highweb-project.github.io/WebVKc-sample/
-	**HTML5 추가 기능 활성화**
	-	크로미엄 내부에 구현된 실험적인 기능들을 사전적으로 사용해 볼 수 있도록 관련 기능들을 활성화 합니다.
-	**DeviceAPI 확장**
	-	자바스크립트 상에서 단말 자원에 접근할 수 있도록 자바스크립트 확장을 제공합니다.
		-	ApplicationLauncher, SystemInformation, Calendar, Contact, Messaging, Gallery, Sensor
	-	각 기능들에 대한 동작 및 샘플 코드는 아래 링크를 참고해 주세요.
		-	https://highweb-project.github.io/DeviceAPI-sample/

크로미엄 브라우저 빌드
----------------------

빌드 절차는 크로미엄 브라우저와 동일합니다. 상세한 크로미엄 브라우저 빌드 가이드는 아래 링크를 참고해 주세요. 이 문서에서는 단순화된 빌드 절차를 명령어와 함께 안내합니다.

https://www.chromium.org/developers/how-tos/get-the-code

빌드에 필요한 사양
------------------

-	우분투 16.04 64비트, 램 8기가 이상, 가급적이면 SSD 권장
-	depot_tools 사전 설정
	-	fetch, gclient 명령어 사용을 위해 사전에 depot_tools가 설정되어 있어야 합니다. 자세한 설정 방법은 아래 링크를 참고하세요.
	-	https://dev.chromium.org/developers/how-tos/install-depot-tools

안드로이드 빌드
---------------

-	하이웹 브라우저 github 저장소를 기준으로 하는 gclient 환경파일 생성<pre><code>$ gclient config --name=src https://github.com/highweb-project/highweb-browser.git@origin/20161214 --unmanaged --deps-file=.DEPS.git</code></pre>

-	안드로이드 관련 소스 파일을 추가 다운로드 받도록 옵션 추가<pre><code>$ echo "target_os = \[\"android\"]" >> .gclient</code></pre>

-	외부 소스 저장소 동기화<pre><code>$ gclient sync -n</code></pre>

-	필요 라이브러리 설치 (최초 1회만 필요)<pre><code>src\$ ./build/install-build-deps-android.sh src$ gclient runhooks</code></pre>

-	GN 빌드 진행 (최초 1회만 필요)

	-	추가 적용 사항에 대해 GN 플래그가 적용되어 있으며 각각 true로 설정해 주어야 기능이 동작합니다.
	-	디버그 버전의 경우 컴포넌트 빌드, 릴리즈 버전의 경우 non-컴포넌트 빌드가 진행됩니다.
	-	빌드 속도 향상을 위해 디버그 버전의 경우 심볼-레벨을 1, 릴리즈 버전은 0으로 설정하였습니다.<pre><code>src\$ gn gen out/AndroidRelease --args='target_os="android" enable_highweb_deviceapi=true enable_highweb_webvkc=true enable_highweb_webcl=true is_debug=false symbol_level=0 proprietary_codecs=true ffmpeg_branding="Chrome"'
	src\$ gn gen out/AndroidDebug --args='target_os="android" enable_highweb_deviceapi=true enable_highweb_webvkc=true enable_highweb_webcl=true is_debug=true symbol_level=1 proprietary_codecs=true ffmpeg_branding="Chrome"'</code></pre>

-	ninja 빌드 진행<pre><code>src$ ninja -C out/AndroidRelease chrome_public_apk</code></pre>

-	APK 설치 및 실행<pre><code>src$ adb install -r out/AndroidRelease/apks/ChromePublic.apk</code></pre>

리눅스 (x64) 빌드
-----------------

-	안드로이드 소스를 다운로드 받고 동기화를 완료했을 경우, 해당 소스 트리에서 그대로 작업 진행하면 됩니다. (소스 1벌로 안드로이드/리눅스/오드로이드 리눅스 빌드 가능)

-	필요 라이브러리 설치 (최초 1회만 필요)<pre><code>src\$ ./build/install-build-deps.sh src$ gclient runhooks</code></pre>

-	GN 빌드 진행 (최초 1회만 필요)<pre><code>src\$ gn gen out/LinuxRelease --args='is_debug=false enable_highweb_deviceapi=true enable_highweb_webvkc=true enable_highweb_webcl=true symbol_level=0 proprietary_codecs=true ffmpeg_branding="Chrome"'
src\$ gn gen out/LinuxDebug --args='is_debug=true enable_highweb_deviceapi=true enable_highweb_webvkc=true enable_highweb_webcl=true symbol_level=1 proprietary_codecs=true ffmpeg_branding="Chrome"'</code></pre>

-	ninja 빌드 진행<pre><code>src$ ninja -C out/LinuxRelease chrome</code></pre>

-	크로미엄 실행  
	<pre><code>src$ ./out/LinuxRelease/chrome
	src\$ ./out/LinuxRelease/chrome --no-sandbox --ignore-gpu-blacklist (기타 오류 발생 시)</code></pre>

오드로이드 리눅스 (x86, arm) 빌드
---------------------------------

-	빌드에 필요한 ARM 관련 라이브러리 설치 (최초 1회만 필요)<pre><code>src$ ./build/install-build-deps.sh --arm</code></pre>
-	ARM sysroot 설치 (최초 1회만 필요)<pre><code>src$ ./build/linux/sysroot_scripts/install-sysroot.py --arch=arm</code></pre>
-	GN 빌드 진행 (최초 1회만 필요)
	- 디버그 빌드 시 용량 문제로 파일 전송에 어려움이 있을 수 있어 릴리즈 빌드를 권장합니다.<pre><code>src$ gn gen out/OdroidRelease --args='target_cpu="arm" use_allocator="none" is_debug=false enable_highweb_deviceapi=true enable_highweb_webvkc=true enable_highweb_webcl=true symbol_level=0 proprietary_codecs=true ffmpeg_branding="Chrome"'</code></pre>
-	ninja 빌드 진행<pre><code>src$ ninja -C out/OdroidRelease chrome</code></pre>
-	오드로이드 파일 전송 후 실행
	-	우분투 16.04 MATE (20161011) 테스트
	-	우분투 16.04의 경우 libgcrypt11 라이브러리 설치 필요<pre><code>wget https://launchpad.net/ubuntu/+archive/primary/+files/libgcrypt11_1.5.3-2ubuntu4.2_armhf.deb
	sudo dpkg -i libgcrypt11_1.5.3-2ubuntu4.2_armhf.deb</code></pre>

Tips
----

-	ccache 적용하기
