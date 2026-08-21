# jitterentropy-android

Android AAR wrapper for
[jitterentropy-library](https://github.com/smuellerDD/jitterentropy-library).
The AAR uses the upstream `arch/android/Android.mk` build template and contains
native binaries for `arm64-v8a`, `armeabi-v7a`, `x86_64`, and `x86`.

## Requirements

- JDK 17 or newer
- Android SDK Platform 36
- Android NDK `28.2.13676358`

Android Studio/Gradle can install the specified NDK when the Android SDK
licenses have already been accepted.

## Build the AAR

Clone with the native dependency and assemble the release variant:

```bash
git submodule update --init --recursive
./gradlew :jitterentropy:assembleRelease
```

At the end of a successful build, Gradle prints the bundled C library version:

```text
jitterentropy-library version: <version>
```

For example,

> ./gradlew :jitterentropy:assembleRelease
>
> Task :jitterentropy:assembleRelease
>
> **jitterentropy-library version: 3.7.1**
>
> BUILD SUCCESSFUL in 616ms

The build command also copies the generated AAR into the root `dist/`
directory so it is easier to locate the build release.

```bash
ls dist/jitterentropy-*-release.aar
```

The artifact version and `JitterEntropy.VERSION_NAME` are derived from the
version macros in the vendored `jitterentropy.h` header during the build.

Confirm that all requested native targets are present:

```bash
unzip -l dist/jitterentropy-*-release.aar \
  | grep 'jni/.*/libjitterentropy_android.so'
```

## Use the AAR

Copy the discovered `jitterentropy-<version>-release.aar` into the `softpos-sdk`
vendor folder's `softpos-sdk/vendor/jitterentropy/<version>/` directory. Replace
`<version>` below with the version from the filename, then add the dependency
to the consuming module (for example, the `crypto` module):

```kotlin
dependencies {
  implementation(files("../vendor/jitterentropy/<version>/jitterentropy-<version>-release.aar"))
}
```

Kotlin:

```kotlin
import eu.npay.android.sdk.softpos.jitterentropy.JitterEntropy

val entropy: ByteArray = JitterEntropy.getRandomEntropy(32)
val version: String = JitterEntropy.VERSION_NAME
```

Java:

```java
import eu.npay.android.sdk.softpos.jitterentropy.JitterEntropy;

byte[] entropy = JitterEntropy.getRandomEntropy(32);
String version = JitterEntropy.VERSION_NAME;
```

Entropy collection is CPU-intensive, so call it away from Android's main
thread. The AAR requires Android 10 (API 29) or newer.
