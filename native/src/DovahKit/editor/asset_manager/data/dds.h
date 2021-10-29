#pragma once
#include <QImage>
#include "_base.h"

namespace DirectX {
   struct ScratchImage;
   struct TexMetadata;
}

class DovahKitAssetDataDDS : public DovahKitAssetData {
   public:
      using DovahKitAssetData::DovahKitAssetData;

      DirectX::ScratchImage* data = nullptr;
      DirectX::TexMetadata*  info = nullptr;
      QImage image;

      virtual bool load(const dovah::bsa_archived_file&) override;
      virtual bool load(dovah::form_stub&) override { return false; }
};