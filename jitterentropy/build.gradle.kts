plugins {
    id("com.android.library")
}

val jitterentropyHeader =
    rootProject.file("vendor/jitterentropy-library/jitterentropy.h")
val jitterentropyHeaderText = jitterentropyHeader.readText()

fun readJitterentropyVersionComponent(macro: String): String {
    val pattern = Regex(
        "(?m)^#define[\\t ]+${Regex.escape(macro)}[\\t ]+([0-9]+)[\\t ]*$",
    )
    return pattern.find(jitterentropyHeaderText)?.groupValues?.get(1)
        ?: error("Cannot read $macro from $jitterentropyHeader")
}

val jitterentropyVersionName = listOf(
    "JENT_MAJVERSION",
    "JENT_MINVERSION",
    "JENT_PATCHLEVEL",
).joinToString(".") { readJitterentropyVersionComponent(it) }

version = jitterentropyVersionName

base {
    archivesName.set("jitterentropy-$jitterentropyVersionName")
}

android {
    namespace = "eu.npay.android.sdk.softpos.jitterentropy"
    compileSdk = 36
    ndkVersion = "28.2.13676358"

    defaultConfig {
        minSdk = 29

        buildConfigField(
            "String",
            "JITTERENTROPY_VERSION_NAME",
            "\"$jitterentropyVersionName\"",
        )

        ndk {
            abiFilters += listOf(
                "arm64-v8a",
                "armeabi-v7a",
                "x86_64",
                "x86",
            )
        }

        consumerProguardFiles("consumer-rules.pro")
    }

    buildFeatures {
        buildConfig = true
    }

    externalNativeBuild {
        ndkBuild {
            path = file("src/main/cpp/Android.mk")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
}

val copyReleaseAarToDist = tasks.register<Copy>("copyReleaseAarToDist") {
    dependsOn("bundleReleaseAar")
    from(
        layout.buildDirectory.file(
            "outputs/aar/jitterentropy-$jitterentropyVersionName-release.aar",
        ),
    )
    into(rootProject.layout.projectDirectory.dir("dist"))
}

tasks.matching { it.name == "assembleRelease" }.configureEach {
    dependsOn(copyReleaseAarToDist)
    doLast {
        println("jitterentropy-library version: $jitterentropyVersionName")
    }
}
