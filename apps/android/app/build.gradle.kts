plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "com.catemup.catchess"

    compileSdk {
        version = release(36) {
            minorApiLevel = 1
        }
    }

    defaultConfig {
        applicationId = "com.catemup.catchess"
        minSdk = 24
        targetSdk = 36
        versionCode = 1
        versionName = "1.0"

        ndk {
            abiFilters += "arm64-v8a"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    packaging {
        resources {
            excludes += setOf(
                "kotlin/**",
                "META-INF/version-control-info.textproto",
                "META-INF/com/android/build/gradle/app-metadata.properties"
            )
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
}
