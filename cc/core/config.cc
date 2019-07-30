// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#include "tink/config.h"

#include "absl/strings/ascii.h"
#include "tink/aead/aead_config.h"
#include "tink/daead/deterministic_aead_config.h"
#include "tink/hybrid/hybrid_config.h"
#include "tink/mac/mac_config.h"
#include "tink/signature/signature_config.h"
#include "tink/streamingaead/streaming_aead_config.h"
#include "tink/util/errors.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "proto/config.pb.h"

using google::crypto::tink::KeyTypeEntry;

namespace crypto {
namespace tink {

// static
std::unique_ptr<google::crypto::tink::KeyTypeEntry>
Config::GetTinkKeyTypeEntry(const std::string& catalogue_name,
                            const std::string& primitive_name,
                            const std::string& key_proto_name,
                            int key_manager_version,
                            bool new_key_allowed) {
  std::string prefix = "type.googleapis.com/google.crypto.tink.";
  std::unique_ptr<KeyTypeEntry> entry(new KeyTypeEntry());
  entry->set_catalogue_name(catalogue_name);
  entry->set_primitive_name(primitive_name);
  entry->set_type_url(prefix.append(key_proto_name));
  entry->set_key_manager_version(key_manager_version);
  entry->set_new_key_allowed(new_key_allowed);
  return entry;
}

// static
crypto::tink::util::Status Config::Validate(const KeyTypeEntry& entry) {
  if (entry.type_url().empty()) {
    return util::Status(util::error::INVALID_ARGUMENT,
                        "Missing type_url.");
  }
  if (entry.primitive_name().empty()) {
    return util::Status(util::error::INVALID_ARGUMENT,
                        "Missing primitive_name.");
  }
  if (entry.catalogue_name().empty()) {
    return util::Status(util::error::INVALID_ARGUMENT,
                        "Missing catalogue_name.");
  }
  return util::Status::OK;
}

// static
util::Status Config::Register(
    const google::crypto::tink::RegistryConfig& config) {
  for (const auto& entry : config.entry()) {
    util::Status status;
    std::string primitive_name = absl::AsciiStrToLower(entry.primitive_name());

    if (primitive_name == "mac") {
      status = MacConfig::Register();
    } else if (primitive_name == "aead") {
      status = AeadConfig::Register();
    } else if (primitive_name == "deterministicaead") {
      status = DeterministicAeadConfig::Register();
    } else if (primitive_name == "hybridencrypt" ||
               primitive_name == "hybriddecrypt") {
      status = HybridConfig::Register();
    } else if (primitive_name == "publickeysign" ||
               primitive_name == "publickeyverify") {
      status = SignatureConfig::Register();
    } else if (primitive_name == "streamingaead") {
      // We don't support catalogues anymore -- we hence simply register
      // everything from the streamingaead config.
      status = StreamingAeadConfig::Register();
    } else {
      status = ToStatusF(crypto::tink::util::error::INVALID_ARGUMENT,
                         "A non-standard primitive '%s' '%s', "
                         "use directly Config::Register<P>(KeyTypeEntry&).",
                         entry.primitive_name().c_str(),
                         primitive_name.c_str()
                         );
    }
    if (!status.ok()) return status;
    status = RegisterWrapper(primitive_name);
    if (!status.ok()) return status;
  }
  return util::Status::OK;
}

// static
util::Status Config::RegisterWrapper(
    absl::string_view lowercase_primitive_name) {
  if (lowercase_primitive_name == "mac") {
    return MacConfig::Register();
  } else if (lowercase_primitive_name == "aead") {
    return AeadConfig::Register();
  } else if (lowercase_primitive_name == "deterministicaead") {
    return DeterministicAeadConfig::Register();
  } else if (lowercase_primitive_name == "hybridencrypt" ||
             lowercase_primitive_name == "hybriddecrypt") {
    return HybridConfig::Register();
  } else if (lowercase_primitive_name == "publickeysign" ||
             lowercase_primitive_name == "publickeyverify") {
    return SignatureConfig::Register();
  } else if (lowercase_primitive_name == "streamingaead") {
    // We don't support catalogues anymore -- we hence simply register
    // everything from the streamingaead config.
    return StreamingAeadConfig::Register();
  } else {
    return crypto::tink::util::Status(
        crypto::tink::util::error::INVALID_ARGUMENT,
        absl::StrCat("Cannot register primitive wrapper for non-standard "
                     "primitive ",
                     lowercase_primitive_name,
                     " (call Registry::RegisterPrimitiveWrapper directly)"));
  }
}

}  // namespace tink
}  // namespace crypto
