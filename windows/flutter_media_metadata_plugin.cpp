/// This file is a part of flutter_media_metadata_ex
/// (https://github.com/alexmercerind/flutter_media_metadata_ex).
///
/// Copyright (c) 2021-2022, Hitesh Kumar Saini <saini123hitesh@gmail.com>.
/// All rights reserved.
/// Use of this source code is governed by MIT license that can be found in the
/// LICENSE file.

#include "include/flutter_media_metadata_ex/flutter_media_metadata_ex_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <future>

#include "../cxx/metadata_retriever.hpp"

namespace {

class FlutterMediaMetadataExPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar);

  FlutterMediaMetadataExPlugin();

  virtual ~FlutterMediaMetadataExPlugin();

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

void FlutterMediaMetadataExPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows* registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "flutter_media_metadata_ex",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<FlutterMediaMetadataExPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto& call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

FlutterMediaMetadataExPlugin::FlutterMediaMetadataExPlugin() {}

FlutterMediaMetadataExPlugin::~FlutterMediaMetadataExPlugin() {}

void FlutterMediaMetadataExPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("MetadataRetriever") == 0) {
    auto arguments = std::get<flutter::EncodableMap>(*method_call.arguments());
    auto file_path =
        std::get<std::string>(arguments[flutter::EncodableValue("filePath")]);
    std::future<void> future = std::async([=, result_ptr =
                                                  std::move(result)]() -> void {
      std::unique_ptr<MetadataRetriever> retriever =
          std::make_unique<MetadataRetriever>();
      retriever->SetFilePath(file_path);
      flutter::EncodableMap metadata;
      for (const auto& [key, value] : *retriever->metadata()) {
        metadata.insert(std::make_pair(flutter::EncodableValue(key),
                                       flutter::EncodableValue(value)));
      }
      flutter::EncodableMap response;
      response.insert(std::make_pair("metadata", metadata));
      if (retriever->album_art() != nullptr)
        response.insert(std::make_pair("albumArt", *retriever->album_art()));
      result_ptr->Success(flutter::EncodableValue(response));
    });
  } else {
    result->NotImplemented();
  }
}

}  // namespace

void FlutterMediaMetadataExPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterMediaMetadataExPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
