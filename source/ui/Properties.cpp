#include "Properties.hpp"

#include "essencio/GameType.hpp"
#include "imgui.h"
#include "Context.hpp"

#include "util/log.hpp"

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void UI::Properties::DrawModel(Loader &loader) {
    for (auto &pair : loader.GetModels()) {
        const essencio::WindowsModel &model = pair.second.data;

        ImGui::Text("Version: %d.%d", model.majorVersion, model.minorVersion);
        ImGui::Text("Min. Bounds: %.3f,%.3f,%.3f",
            model.boundsMin.x, model.boundsMin.y, model.boundsMin.z);
        ImGui::Text("Max. Bounds: %.3f,%.3f,%.3f",
            model.boundsMax.x, model.boundsMax.y, model.boundsMax.z);

        // TODO: Add extra parameter info

        for (uint32_t i = 0; i < model.rigs.size(); ++i) {
            const auto &rig = model.rigs[i];
            std::string rigLabel = "Rig #" + std::to_string(i + 1);

            if (ImGui::CollapsingHeader(rigLabel.c_str())) {
                ImGui::Indent();
                ImGui::Text("# of Bones: %d", rig.numBones);
                ImGui::Unindent();
            }
        }

        for (uint32_t i = 0; i < model.meshes.size(); ++i) {
            // Native mesh data loaded through Essencio
            const auto &mesh = model.meshes[i];
            // Custom loaded data from Loader
            auto &meshData = pair.second.meshes[i];

            std::string meshLabel = "Mesh #" + std::to_string(i + 1);
            if (ImGui::CollapsingHeader(meshLabel.c_str())) {
                ImGui::Indent();

                std::string isVisibleLabel = "Is Visible##" + std::to_string(i);
                ImGui::Checkbox(isVisibleLabel.c_str(), &meshData.isVisible);

                for (uint32_t j = 0; j < meshData.materials.size(); ++j) {
                    TextureHandle texture = meshData.handle.textures[j];

                    std::string materialLabel = "Material #" + std::to_string(j + 1) + "##" + std::to_string(i);
                    if (ImGui::CollapsingHeader(materialLabel.c_str())) {
                        ImGui::Indent();

                        bool isCurrent = (meshData.materialIndex == j);
                        std::string isCurrentLabel = "Is Current##" + std::to_string(i) + "_" + std::to_string(j);
                        if (ImGui::Checkbox(isCurrentLabel.c_str(), &isCurrent)) {
                            if (isCurrent) {
                                meshData.materialIndex = j;
                            }
                        }

                        if (texture != 0) {
                            std::string materialButtonLabel = "MaterialButton##" + std::to_string(i) + "_" + std::to_string(j);
                            if (ImGui::ImageButton(materialButtonLabel.c_str(), texture, ImVec2(128, 128))) {
                                Context::Get().SetNextFile(meshData.materials[0]->path.c_str());
                            }
                        }

                        ImGui::Unindent();
                    }

                }

                ImGui::Text("Min. Bounds: %.3f,%.3f,%.3f",
                    mesh.boundsMin.x, mesh.boundsMin.y, mesh.boundsMin.z);
                ImGui::Text("Max. Bounds: %.3f,%.3f,%.3f",
                    mesh.boundsMax.x, mesh.boundsMax.y, mesh.boundsMax.z);

                ImGui::Text("# of Vertices: %d", mesh.numVertices);
                ImGui::Text("# of Faces: %d", mesh.numFaces);
                ImGui::Text("# of Vertex Keys: %d", mesh.numVertexKeys);

                for (uint32_t j = 0; j < mesh.vertexKeys.size(); ++j) {
                    const auto &key = mesh.vertexKeys[j];
                    std::string keyLabel = "Vertex Key #" + std::to_string(j + 1)
                        + "##mesh" + std::to_string(i) + "_key" + std::to_string(j);

                    if (ImGui::CollapsingHeader(keyLabel.c_str())) {
                        ImGui::Indent();
                        std::string type;

                        switch (key.type) {
                            case essencio::VertexKeyType::FLOAT2: type = "FLOAT2"; break;
                            case essencio::VertexKeyType::FLOAT3: type = "FLOAT3"; break;
                            case essencio::VertexKeyType::FLOAT: type = "FLOAT"; break;
                            case essencio::VertexKeyType::UNKNOWN: type = "(Unknown)"; break;
                        }

                        ImGui::Text("Offset: %d", key.offset);
                        ImGui::Text("Type: %s", type.c_str());
                        ImGui::Text("Index: %d", key.index);
                        ImGui::Text("Sub Index: %d", key.subIndex);

                        ImGui::Unindent();
                    }
                }

                ImGui::Unindent();
            }
        }
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void UI::Properties::DrawMaterial(Loader &loader) {
    const auto &gameType = Context::Get().GetGameType();

    for (const auto &material : loader.GetMaterials()) {
        const auto &data = material.second.data;

        if (gameType == essencio::GameType::MYSIMS) {
            ImGui::Text("Header Size: %d", data.headerSize);
            ImGui::Text("Total Size: %d", data.totalSize);
            ImGui::Text("Version: %d", data.version);

            ImGui::Text("Material Hash: 0x%X", data.materialHash);
            ImGui::Text("Shader Hash: 0x%X", data.shaderHash);
        }

        ImGui::Text("Data Size: %d", data.dataSize);

        ImGui::Text("Parameter Size: %d", data.paramSize);
        ImGui::Text("# of Parameters: %d", data.paramCount);

        for (uint32_t i = 0; i < data.params.size(); ++i) {
            const auto &param = data.params[i];
            const std::string paramLabel = "Parameter #" + std::to_string(i + 1);

            if (ImGui::CollapsingHeader(paramLabel.c_str())) {
                ImGui::Indent();
                std::string valueType;

                switch (param.valueType) {
                    case essencio::MaterialParameterType::COLOR: valueType = "COLOR"; break;
                    case essencio::MaterialParameterType::VALUE: valueType = "VALUE"; break;
                    case essencio::MaterialParameterType::RESOURCE_KEY: valueType = "RESOURCE_KEY"; break;
                }

                std::string typeName;

                // This definitely needs cleaning up... but it works for now
                // I sure love reflection in C++
                if (gameType == essencio::GameType::KINGDOM) {
                    switch (param.type) {
                        case 0x7fee2d1a: typeName = "diffuseColor"; break;
                        case 0x76f88689: typeName = "useLights"; break;
                        case 0x2616b09a: typeName = "highlightMultiplier"; break;
                        case 0x6cc0fd85: typeName = "diffuseMap"; break;
                        case 0x20cb22b7: typeName = "ambientMap"; break;
                        case 0xad528a60: typeName = "specularMap"; break;
                        case 0xf46b90ae: typeName = "shadowReceiver"; break;
                        case 0xb2649c2f: typeName = "blendmode"; break;
                        default: typeName = "(Unknown)"; break;
                    }
                } else {
                    switch (param.type) {
                        case 0x7fee2d1a: typeName = "diffuseColor"; break;
                        case 0x76f88689: typeName = "useLights"; break;
                        case 0x2616b09a: typeName = "highlightMultiplier"; break;
                        case 0x6cc0fd85: typeName = "diffuseMap"; break;
                        case 0x20cb22b7: typeName = "ambientMap"; break;
                        case 0xad528a60: typeName = "specularMap"; break;
                        case 0x2a20e51b: typeName = "alphaMap"; break;
                        case 0xf46b90ae: typeName = "shadowReceiver"; break;
                        case 0xb2649c2f: typeName = "blendmode"; break;
                        case 0x05d22fd3: typeName = "transparency"; break;
                        case 0x04a5daa3: typeName = "ambient"; break;
                        case 0x637daa05: typeName = "diffuse"; break;
                        case 0xd1f4cb96: typeName = "greenChannelMultiplier"; break;
                        case 0x7bb10c17: typeName = "blueChannelMultiplier"; break;
                        case 0x99bf82f6: typeName = "redChannelMultiplier"; break;
                        case 0x689aeffe: typeName = "nightTint"; break;
                        case 0xfbbbb5c2: typeName = "dayTint"; break;
                        case 0x1d17d10f: typeName = "overbrightDay"; break;
                        case 0xdb88ec28: typeName = "negativeColorBiasNight"; break;
                        case 0x29214c0c: typeName = "negativeColorBiasDay"; break;
                        case 0xb779f79b: typeName = "overbrightNight"; break;
                        case 0xbf2ad9b3: typeName = "specularColor"; break;
                        case 0x2ce11842: typeName = "specular"; break;
                        case 0x988403f9: typeName = "transparent"; break;
                        case 0xab26e148: typeName = "vNormalWaveSpeed"; break;
                        case 0xf303d152: typeName = "emissionMap"; break;
                        case 0xdb319586: typeName = "vReflectionWaveSpeed"; break;
                        case 0x3c45e334: typeName = "normalMapScale"; break;
                        case 0xa2e40eab: typeName = "jitterScale"; break;
                        case 0x02937388: typeName = "waveFrequency"; break;
                        case 0x50e0193b: typeName = "uReflectionWaveSpeed"; break;
                        case 0x2a93bafb: typeName = "waterColorBlue"; break;
                        case 0x5916ed3e: typeName = "baseAlpha"; break;
                        case 0xe460597b: typeName = "reflectionSharpness"; break;
                        case 0x933e38f4: typeName = "intensity"; break;
                        case 0x11efe2fd: typeName = "waveAmplitude"; break;
                        case 0x7fd42f11: typeName = "noiseFrequency"; break;
                        case 0xbd237b0d: typeName = "ShinyPower"; break;
                        case 0x2e18b549: typeName = "VspeedLayer2"; break;
                        case 0xdb5ebee7: typeName = "warpAmp"; break;
                        case 0x2e18b54b: typeName = "VspeedLayer0"; break;
                        case 0x2e18b54a: typeName = "VspeedLayer1"; break;
                        case 0x7eea0c2b: typeName = "UspeedLayer1"; break;
                        case 0x7eea0c2a: typeName = "UspeedLayer0"; break;
                        case 0x7eea0c28: typeName = "UspeedLayer2"; break;
                        case 0xd552a779: typeName = "reflectionIntensity"; break;
                        case 0xb32a1342: typeName = "reflectionAmount"; break;
                        case 0x9f63578d: typeName = "uNormalWaveSpeed"; break;
                        case 0xf72fca9b: typeName = "diffuseAlpha"; break;
                        case 0x7490c750: typeName = "contrastSubtractColor"; break;
                        case 0x6612378c: typeName = "contrastMultiplyColor"; break;
                        case 0x9038f94b: typeName = "amBodyShakespeare"; break;
                        case 0x1067900c: typeName = "amHeadHairLongSpikey"; break;
                        case 0x0923fb40: typeName = "auHeadHairBigFro"; break;
                        case 0x58b2f06d: typeName = "afBodyLayeredSkirt"; break;
                        case 0x80c83701: typeName = "afHeadHairFortune"; break;
                        case 0x75486bde: typeName = "amHeadHairSpikey"; break;
                        case 0x6c8c62c9: typeName = "afHeadHairTightBun"; break;
                        case 0x61f36b5b: typeName = "afHeadHatPirateGinny"; break;
                        case 0xe17e380c: typeName = "auHeadHatCap"; break;
                        case 0x0fdc6fdc: typeName = "faceSkinTones"; break;
                        case 0x9eda8cf5: typeName = "auHeadHairFlowerCrown"; break;
                        case 0xf0d0e420: typeName = "amHeadHatBellhop"; break;
                        case 0xc1519bcf: typeName = "amHeadHatMagician"; break;
                        case 0x8f0c0492: typeName = "auHeadHatPilotGoggles"; break;
                        case 0xc5ae022b: typeName = "afBodyLowPoofSkirt"; break;
                        case 0x383b9128: typeName = "afBodyMayor"; break;
                        case 0xd89ad4d5: typeName = "auHeadHatCapback"; break;
                        case 0x7255e7be: typeName = "afHeadHairLibrarian"; break;
                        case 0xe2498117: typeName = "afBodyTurtleneckBaggy"; break;
                        case 0xbcb6f07c: typeName = "auHeadHatBeenie"; break;
                        case 0xfcdf8c6a: typeName = "afHeadHairSmallBraids"; break;
                        case 0x359839d2: typeName = "afHeadHairPuffyLayers"; break;
                        case 0xde545f5e: typeName = "amHeadHairIvyLeague"; break;
                        case 0x146cb6b6: typeName = "afHeadHairMayor"; break;
                        case 0xe97a9352: typeName = "amHeadHairNigel"; break;
                        case 0xb4de4520: typeName = "auHeadHatNinja"; break;
                        case 0x7c22b02c: typeName = "auHeadHairMidShaggy"; break;
                        case 0x556e4212: typeName = "afBodyShortApron"; break;
                        case 0x8cbf470e: typeName = "afHeadHairCurlsRibbons"; break;
                        case 0x3b2679d5: typeName = "auBodyPantsJacketBag"; break;
                        case 0xd12b0c98: typeName = "afBodyShortSkirtSweater"; break;
                        case 0xcf76a1c7: typeName = "amHeadHairRay"; break;
                        case 0xe029e90d: typeName = "amHeadHairArcade"; break;
                        case 0xf434aa77: typeName = "afBodyHighPoofLongSkirt"; break;
                        case 0xea080c69: typeName = "afHeadHatBandanaDreads"; break;
                        case 0xc9314483: typeName = "auHeadHairFoxEars"; break;
                        case 0x7d6fdc4c: typeName = "afBodyCollarSkirt"; break;
                        case 0xc51bb766: typeName = "afBodyCoatSkirt"; break;
                        case 0xe806c452: typeName = "afHeadHairStylishPeacock"; break;
                        case 0x5f00b265: typeName = "afBodyKimono"; break;
                        case 0x16e4ca30: typeName = "auHeadHatTopHat"; break;
                        case 0xb7a93aa8: typeName = "amHeadHatChef"; break;
                        case 0x6515eb2c: typeName = "auBodyKnight"; break;
                        case 0xe9ca3e0b: typeName = "amHeadHairEthan"; break;
                        case 0x0a371797: typeName = "afHeadHairClara"; break;
                        case 0x6e2b178d: typeName = "afHeadHatWendalyn"; break;
                        case 0xa822b3e8: typeName = "amBodyHauntedHouseBoy"; break;
                        case 0xd59381fb: typeName = "auHeadHatMohawk"; break;
                        case 0xe72a5f1c: typeName = "auHeadHairSuperShortLayered"; break;
                        case 0xeb85d831: typeName = "amHeadHairTim"; break;
                        case 0x79a3b7ff: typeName = "auBodyHoodiePants"; break;
                        case 0xd01b0a09: typeName = "afBodyLongSleeveLongDress"; break;
                        case 0x4a351bdc: typeName = "afHeadHatCowgirl"; break;
                        case 0x9ed158ab: typeName = "auHeadHatBald"; break;
                        case 0xd9dfb575: typeName = "amBodyMartialArts"; break;
                        case 0xe2b571a9: typeName = "propBookClosed"; break;
                        case 0xe1a22a57: typeName = "amBodyFlipper"; break;
                        case 0x66ba9c80: typeName = "afBodyLongSkirtLargeCuff"; break;
                        case 0xe4f0d787: typeName = "auHeadHatPirate"; break;
                        case 0x26f07855: typeName = "auHeadHairShortFlatBangs"; break;
                        case 0x2836ea65: typeName = "auBodyBellhop"; break;
                        case 0xc800d94b: typeName = "auBodyApronBear"; break;
                        case 0xdd91b6b6: typeName = "afBodyKneeLengthSkirt"; break;
                        case 0x8d58d24f: typeName = "auHeadHatRasta"; break;
                        case 0x32e0ca0b: typeName = "afBodyLongSkirt"; break;
                        case 0x23c6d774: typeName = "auBodySkinTight"; break;
                        case 0x7bebdd19: typeName = "auBodyCalfLengthPants"; break;
                        case 0xeddcece1: typeName = "plumbobColor"; break;
                        case 0x455bef77: typeName = "afHeadHairDoubleBuns"; break;
                        case 0xc36d202b: typeName = "auHeadHairHairspraySpikey"; break;
                        case 0xafc8f11b: typeName = "afHeadHairRaveKimono"; break;
                        case 0x40a202a7: typeName = "auHeadHairBowlCut"; break;
                        case 0xad6d2254: typeName = "amHeadHairCruise"; break;
                        case 0x57059004: typeName = "auBodyLongPantsBoots"; break;
                        case 0x791597ca: typeName = "afHeadHatNewspaperCap"; break;
                        case 0x5519cfb6: typeName = "afHeadHatBandana"; break;
                        case 0x811f207f: typeName = "afHeadHairAlexa"; break;
                        case 0x37c3b76c: typeName = "afHeadHairStreakedLolita"; break;
                        case 0xf8404ffa: typeName = "afHeadHairPuffyLayersBunny"; break;
                        case 0xbe323f01: typeName = "auBodyApronTshirtPants"; break;
                        case 0xc34a68d0: typeName = "auBodyLongPantsShortSleeves"; break;
                        case 0xbcf4239b: typeName = "amBodyArcade"; break;
                        case 0xb3f9d3f1: typeName = "afBodyAlexa"; break;
                        case 0xcb6a2c62: typeName = "afBodyAsymmetricalSkirt"; break;
                        case 0x88b04723: typeName = "auHeadHatCadet"; break;
                        case 0x8157dc19: typeName = "auBodyBear"; break;
                        case 0x4e053dbd: typeName = "auHeadHairDisco"; break;
                        case 0xaf284852: typeName = "afBodyShortSleeveApron"; break;
                        case 0x8804b9b4: typeName = "auBodyRolledSleevesLongPants"; break;
                        case 0x4487e3d4: typeName = "afHeadHairPigTail"; break;
                        case 0xebbb243f: typeName = "afHeadHairLooseCurlsLong"; break;
                        case 0xbbf23c58: typeName = "afHeadHairLong"; break;
                        case 0xb9642ff0: typeName = "afHeadHairPigTailFlowers"; break;
                        case 0xaa3cd006: typeName = "afHeadHairLongBraid"; break;
                        case 0x804ad79a: typeName = "afHeadHairLongPigtail"; break;
                        case 0x608caa94: typeName = "afHeadHatBeaniePigtails"; break;
                        case 0xc3dd71da: typeName = "afBodyChineseDress"; break;
                        case 0xd987c7ad: typeName = "amHeadHatCowboy"; break;
                        case 0x667d4e9c: typeName = "afBodyFloristYoungerSister"; break;
                        case 0x1688f273: typeName = "auHeadHatEarhat"; break;
                        case 0x4038b561: typeName = "afHeadHairHighSalon"; break;
                        case 0xb857a450: typeName = "afHeadHairSoftBobBangs"; break;
                        case 0x16229f12: typeName = "afHeadHairKarine"; break;
                        case 0xe808f034: typeName = "amBodyGothCoat"; break;
                        case 0xebb1363d: typeName = "afHeadHairBangsHighPonyTail"; break;
                        case 0x10b11928: typeName = "amHeadHatMartialArts"; break;
                        case 0x31b41a58: typeName = "auHeadHairShortFro"; break;
                        case 0x32ff6934: typeName = "afBodyWendalyn"; break;
                        case 0x172754f2: typeName = "amHeadHatShakespeare"; break;
                        case 0x4cf48f41: typeName = "auBodyBackpack"; break;
                        case 0x5e5e0bb5: typeName = "auHeadHairLongLayered"; break;
                        case 0xdb272b16: typeName = "afHeadHairBee"; break;
                        case 0x0562a36e: typeName = "amHeadHairSlickBack"; break;
                        case 0xadf12cdc: typeName = "afHeadHatFedoraHeadset"; break;
                        case 0x206508d6: typeName = "auBodySuitBowTie"; break;
                        case 0x21eaeac7: typeName = "amHeadHatNewspaperCap"; break;
                        case 0x0568e523: typeName = "auBodyNoSleevesLongPants"; break;
                        case 0x4c34687c: typeName = "amHeadHairPompadour"; break;
                        case 0x1f8c55e8: typeName = "auBodyPirate"; break;
                        case 0x6293ca92: typeName = "auBodyShortPantsShortSleeves"; break;
                        case 0x57cffd77: typeName = "amBodyChef"; break;
                        case 0x9e77273b: typeName = "afBodyShortSkirtBag"; break;
                        case 0x407405f3: typeName = "afHeadHairPuffyLayersTiara"; break;
                        case 0x61ab1e17: typeName = "amHeadHairSidePart"; break;
                        case 0x69e3e1a7: typeName = "amBodySamurai"; break;
                        case 0xf67d2839: typeName = "amHeadHairShort"; break;
                        case 0x68592352: typeName = "amHeadHatFlipper"; break;
                        case 0x1437614b: typeName = "afBodyFortuneTeller"; break;
                        case 0xa8ec7cf0: typeName = "auBodyShortSleevApronPants"; break;
                        case 0x7fd2fd6d: typeName = "auHeadHatVisor"; break;
                        case 0x94f2a3f5: typeName = "auHeadHairMuseum"; break;
                        case 0xab532e19: typeName = "amHeadHatBuzzCut"; break;
                        case 0xeae93c5d: typeName = "auBodyShortJacketClosed"; break;
                        case 0x115e90e5: typeName = "afBodyHighPoofSkirt"; break;
                        case 0xa66e30f0: typeName = "auHeadHatHippieBandana"; break;
                        case 0xe042258b: typeName = "afBodySmallWings"; break;
                        case 0x6c25c854: typeName = "afHeadHairStylish"; break;
                        case 0x30872f06: typeName = "afHeadHairLayeredBangsMedium"; break;
                        case 0x6a7c3efc: typeName = "afHeadHatCrumplebottom"; break;
                        case 0x0a0bf0f7: typeName = "amHeadHatFedora"; break;
                        case 0x5c211319: typeName = "auHeadHairDandelion"; break;
                        case 0x1514f851: typeName = "auBodyNinja"; break;
                        case 0x018261bd: typeName = "auHeadHairSushi"; break;
                        case 0x2a45864c: typeName = "afHeadHairLongLayered"; break;
                        case 0x22df72ce: typeName = "afHeadHairLongPonytail"; break;
                        case 0x5a0c9575: typeName = "auHeadHairVeryShortSpiky"; break;
                        case 0x8d6f69f2: typeName = "afHeadHairObservatory"; break;
                        case 0x8637df43: typeName = "auBodyBulkySweaterLongPants"; break;
                        case 0x5fc9d348: typeName = "auHeadHairShortCurly"; break;
                        case 0x86769daf: typeName = "auBodyLongPantsLongSleeve"; break;
                        case 0x2344a259: typeName = "amHeadHairScientist"; break;
                        case 0x79dbab9e: typeName = "auHeadHatBear"; break;
                        case 0x1f0050d9: typeName = "amHeadHatCombOver"; break;
                        case 0x04985945: typeName = "afBodyMini"; break;
                        case 0x37a80fc1: typeName = "afHeadHairUpdoRibbons"; break;
                        case 0x3d88b8b3: typeName = "amHeadHatBaker"; break;
                        case 0xbcc02d91: typeName = "auBodyLabCoat"; break;
                        case 0x0a345310: typeName = "envmapAlpha"; break;
                        case 0x4a1a7937: typeName = "door_archedTopBar"; break;
                        default: typeName = "(Unknown)"; break;
                    }
                }

                if (!typeName.empty()) {
                    ImGui::Text("Type: %s", typeName.c_str());
                } else {
                    ImGui::Text("Type: 0x%X", param.type);
                }

                ImGui::Text("Value Type: %s", valueType.c_str());
                ImGui::Text("# of Value Fields: %d", param.valueFieldCount);
                ImGui::Text("Offset: %d", param.offset);

                switch (param.valueType) {
                    case essencio::MaterialParameterType::COLOR:
                        {
                            std::string colorLabel = "Color##" + std::to_string(i);
                            if (ImGui::CollapsingHeader(colorLabel.c_str())) {
                                ImGui::Indent();

                                for (const auto &channel : param.color) {
                                    ImGui::Text("Channel: %.f", channel);
                                }

                                ImGui::Unindent();
                            }
                        }
                        break;
                    case essencio::MaterialParameterType::VALUE:
                        {
                            std::string valueLabel = "Value##" + std::to_string(i);
                            if (ImGui::CollapsingHeader(valueLabel.c_str())) {
                                ImGui::Indent();
                                ImGui::Text("Value: %d", param.value);
                                ImGui::Unindent();
                            }
                        }
                        break;
                    case essencio::MaterialParameterType::RESOURCE_KEY:
                        {
                            std::string resourceKeyLabel = "ResourceKey##" + std::to_string(i);
                            if (ImGui::CollapsingHeader(resourceKeyLabel.c_str())) {
                                ImGui::Indent();

                                ImGui::Text("Type: 0x%X", param.mapKey.type);
                                ImGui::Text("Group: 0x%X", param.mapKey.group);
                                ImGui::Text("Instance: 0x%llX", param.mapKey.instance);

                                ImGui::Unindent();
                            }
                        }
                        break;
                }

                ImGui::Unindent();
            }
        }
    }
}

void UI::Properties::Draw() {
    ImGui::Begin("Properties");

    auto &loader = Context::Get().GetLoader();

    switch (Context::Get().GetContextType()) {
        case ContextType::MODEL:
            DrawModel(loader);
            break;
        case ContextType::MATERIAL:
            DrawMaterial(loader);
            break;
        default:
            break;
    }

    ImGui::End();
}

