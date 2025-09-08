cmake_minimum_required(VERSION 3.21)

# =========================
# Project
# =========================
project(AndroidVoiceChanger
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================
# JUCE の取得
#   1) 既にサブモジュール等で存在→ add_subdirectory(JUCE) する
#   2) JUCE_DIR が指定されている→ add_subdirectory(${JUCE_DIR})
#   3) 上記が無い→ FetchContent で GitHub から取得
# =========================
include(FetchContent)

# ユーザーが -DJUCE_DIR=/path/to/JUCE を与えた場合を優先
if(DEFINED JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
  message(STATUS "Using JUCE from JUCE_DIR=${JUCE_DIR}")
  add_subdirectory(${JUCE_DIR} JUCE)
elseif(EXISTS "${CMAKE_SOURCE_DIR}/JUCE/CMakeLists.txt")
  message(STATUS "Using JUCE from local subdirectory JUCE/")
  add_subdirectory(${CMAKE_SOURCE_DIR}/JUCE JUCE)
else()
  message(STATUS "Fetching JUCE via FetchContent (v7.0.9)…")
  FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.9
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(juce)
endif()

# =========================
# 生成物の設定
# =========================
# 出力ディレクトリを分けたい場合は有効化
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# =========================
# Plugin ターゲット
# =========================
juce_add_plugin(AndroidVoiceChanger
  PRODUCT_NAME "AndroidVoiceChanger"
  COMPANY_NAME "YourCompany"
  BUNDLE_ID com.example.androidvoicechanger
  COPY_PLUGIN_AFTER_BUILD TRUE

  # フォーマット（プラットフォームに応じて自動無視されます）
  LV2 FALSE
  VST3 TRUE
  AU TRUE
  AUv3 TRUE
  AAX FALSE
  Standalone TRUE

  # iOS/Android 用
  IS_MIDI_EFFECT FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE

  # サンドボックス無効（必要に応じて）
  MICROPHONE_PERMISSION_ENABLED TRUE

  # ここではデフォルトの入出力: ステレオ
  # モノラルにしたい場合は isBusesLayoutSupported を調整
)

# =========================
# ソースの登録
# =========================
target_sources(AndroidVoiceChanger PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginProcessor.h

  # 既存の Editor を使う場合は含めてOK（GenericEditor運用でも害はありません）
  Source/PluginEditor.cpp
  Source/PluginEditor.h

  # DSP
  Source/Dsp/GranularPitchShifter.h
  Source/Dsp/GranularPitchShifter.cpp
  Source/Dsp/Formant.h
  Source/Dsp/Formant.cpp
  Source/Dsp/Tilt.h
  Source/Dsp/Tilt.cpp
)

# =========================
# 依存モジュール
# =========================
target_link_libraries(AndroidVoiceChanger
  PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# =========================
# プラットフォーム別オプション
# =========================
if(APPLE)
  # AU を有効化するには Xcode の署名設定が必要な場合があります
  # set_target_properties(AndroidVoiceChanger PROPERTIES XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
endif()

if(WIN32)
  # 文字コードや互換性用の定義など
  target_compile_definitions(AndroidVoiceChanger PRIVATE
    _CRT_SECURE_NO_WARNINGS
  )
endif()

if(ANDROID)
  # ネイティブオーディオ最適化などを行いたい場合はここに追記
  # 例: target_compile_options(AndroidVoiceChanger PRIVATE -Ofast)
endif()

# =========================
# 開発便利オプション
# =========================
# プラグインのバージョンなどを C++ 側で参照したい場合
target_compile_definitions(AndroidVoiceChanger PRIVATE
  APP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  APP_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  APP_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# =========================
# 追加: リソース (必要なら)
# =========================
# juce_add_binary_data(Resources
#   SOURCES
#     Assets/whatever.png
# )
# target_link_libraries(AndroidVoiceChanger PRIVATE Resources)
