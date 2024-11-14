#include "ModSource.hpp"

#include <Geode/loader/ModMetadata.hpp>
#include <Geode/loader/ModSettingsManager.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <server/DownloadManager.hpp>
#include <Geode/binding/GameObject.hpp>
#include <unordered_set>

LoadModSuggestionTask loadModSuggestion(LoadProblem const& problem) {
    // Recommended / suggested are essentially the same thing for the purposes of this
    if (problem.type == LoadProblem::Type::Recommendation || problem.type == LoadProblem::Type::Suggestion) {
        auto suggestionID = problem.message.substr(0, problem.message.find(' '));
        auto suggestionVersionStr = problem.message.substr(problem.message.find(' ') + 1);

        if (auto suggestionVersionRes = ComparableVersionInfo::parse(suggestionVersionStr)) {
            server::ModVersion suggestionVersion = server::ModVersionLatest();
            if (suggestionVersionRes.unwrap().getComparison() == VersionCompare::MoreEq) {
                suggestionVersion = server::ModVersionMajor {
                    .major = suggestionVersionRes.unwrap().getUnderlyingVersion().getMajor()
                };
            }
            // todo: if mods are allowed to specify other type of version comparisons in the future, 
            // add support for that here
            
            if (auto mod = std::get_if<Mod*>(&problem.cause)) {
                return server::getModVersion(suggestionID, suggestionVersion).map(
                    [mod = *mod](auto* result) -> LoadModSuggestionTask::Value {
                        if (result->isOk()) {
                            return ModSuggestion {
                                .suggestion = result->unwrap().metadata,
                                .forMod = mod,
                            };
                        }
                        return std::nullopt;
                    }
                );
            }
        }
    }
    return LoadModSuggestionTask::immediate(std::nullopt);
}

ModSource::ModSource(Mod* mod) : m_value(mod) {}
ModSource::ModSource(server::ServerModMetadata&& metadata) : m_value(metadata) {}

std::string ModSource::getID() const {
    return std::visit(makeVisitor {
        [](Mod* mod) {
            return mod->getID();
        },
        [](server::ServerModMetadata const& metadata) {
            return metadata.id;
        },
    }, m_value);
}
ModMetadata ModSource::getMetadata() const {
    return std::visit(makeVisitor {
        [](Mod* mod) {
            return mod->getMetadata();
        },
        [](server::ServerModMetadata const& metadata) {
            // Versions should be guaranteed to have at least one item
            return metadata.versions.front().metadata;
        },
    }, m_value);
}

std::string ModSource::formatDevelopers() const {
    return std::visit(makeVisitor {
        [](Mod* mod) {
            return ModMetadata::formatDeveloperDisplayString(mod->getMetadata().getDevelopers());
        },
        [](server::ServerModMetadata const& metadata) {
            // Versions should be guaranteed to have at least one item
            return metadata.formatDevelopersToString();
        },
    }, m_value);
}

CCNode* ModSource::createModLogo() const {
    return std::visit(makeVisitor {
        [](Mod* mod) {
            return geode::createModLogo(mod);
        },
        [](server::ServerModMetadata const& metadata) {
            return createServerModLogo(metadata.id);
        },
    }, m_value);
}
bool ModSource::wantsRestart() const {
    // If some download has been done for this mod, always want a restart
    auto download = server::ModDownloadManager::get()->getDownload(this->getID());
    if (download && download->isDone()) {
        return true;
    }
    return std::visit(makeVisitor {
        [](Mod* mod) {
            return mod->getRequestedAction() != ModRequestedAction::None ||
                ModSettingsManager::from(mod)->restartRequired();
        },
        [](server::ServerModMetadata const& metdata) {
            return false;
        },
    }, m_value);
}
std::optional<server::ServerModUpdate> ModSource::hasUpdates() const {
    return m_availableUpdate;
}

ModSource ModSource::convertForPopup() const {
    return std::visit(makeVisitor {
        [](Mod* mod) {
            return ModSource(mod);
        },
        [](server::ServerModMetadata const& metadata) {
            if (auto mod = Loader::get()->getInstalledMod(metadata.id)) {
                return ModSource(mod);
            }
            return ModSource(server::ServerModMetadata(metadata));
        },
    }, m_value);
}

Mod* ModSource::asMod() const {
    auto mod = std::get_if<Mod*>(&m_value);
    return mod ? *mod : nullptr;
}
server::ServerModMetadata const* ModSource::asServer() const {
    return std::get_if<server::ServerModMetadata>(&m_value);
}

server::ServerRequest<std::optional<std::string>> ModSource::fetchAbout() const {
    // todo: write as visit
    if (auto mod = this->asMod()) {
        return server::ServerRequest<std::optional<std::string>>::immediate(Ok(mod->getMetadata().getDetails()));
    }
    return server::getMod(this->getID()).map(
        [](auto* result) -> Result<std::optional<std::string>, server::ServerError> {
            if (result->isOk()) {
                return Ok(result->unwrap().about);
            }
            return Err(result->unwrapErr());
        }
    );
}
server::ServerRequest<std::optional<std::string>> ModSource::fetchChangelog() const {
    if (auto mod = this->asMod()) {
        return server::ServerRequest<std::optional<std::string>>::immediate(Ok(mod->getMetadata().getChangelog()));
    }
    return server::getMod(this->getID()).map(
        [](auto* result) -> Result<std::optional<std::string>, server::ServerError> {
            if (result->isOk()) {
                return Ok(result->unwrap().changelog);
            }
            return Err(result->unwrapErr());
        }
    );
}
server::ServerRequest<server::ServerModMetadata> ModSource::fetchServerInfo() const {
    // Request the info even if this is already a server mod because this might 
    // not have the full details (for example changelog) and the server cache 
    // should deal with performance issues
    return server::getMod(this->getID());
}
server::ServerRequest<std::unordered_set<std::string>> ModSource::fetchValidTags() const {
    return std::visit(makeVisitor {
        [](server::ServerModMetadata const& metadata) {
            // Server info tags are always certain to be valid since the server has already validated them
            return server::ServerRequest<std::unordered_set<std::string>>::immediate(Ok(metadata.tags));
        },
        [this](auto const&) {
            return server::getTags().map(
                [modTags = this->getMetadata().getTags()](auto* result) -> Result<std::unordered_set<std::string>, server::ServerError> {
                    if (result->isOk()) {
                        std::unordered_set<std::string> fetched = result->unwrap();
                        // Filter out invalid tags
                        auto finalTags = std::unordered_set<std::string>();
                        for (auto& tag : modTags) {
                            if (result->unwrap().contains(tag)) {
                                finalTags.insert(tag);
                            }
                        }
                    }

                    return Ok(finalTags);
                },
                [](server::ServerProgress* progress) {
                    return *progress;
                }
            );
        },
    }, m_value);
}
server::ServerRequest<std::optional<server::ServerModUpdate>> ModSource::checkUpdates() {
    m_availableUpdate = std::nullopt;
    return std::visit(makeVisitor {
        [this](Mod* mod) {
            return server::checkUpdates(mod).map(
                [this](auto* result) -> Result<std::optional<server::ServerModUpdate>, server::ServerError> {
                    if (result->isOk()) {
                        m_availableUpdate = result->unwrap();
                        return Ok(m_availableUpdate);
                    }
                    return Err(result->unwrapErr());
                }
            );
        },
        [](server::ServerModMetadata const& metadata) {
            // Server mods aren't installed so you can't install updates for them
            return server::ServerRequest<std::optional<server::ServerModUpdate>>::immediate(Ok(std::nullopt));
        },
    }, m_value);
}
void ModSource::startInstall() {
    if (auto updates = this->hasUpdates()) {
        if (updates->replacement.has_value()) {
            server::ModDownloadManager::get()->startDownload(
                updates->replacement->id,
                updates->replacement->version,
                std::nullopt,
                this->getID()
            );
        } else {
            server::ModDownloadManager::get()->startDownload(
                this->getID(),
                updates->version
            );
        }
    } else {
        server::ModDownloadManager::get()->startDownload(
            this->getID(),
            this->asServer()
                ? std::optional{this->asServer()->latestVersion().getVersion()}
                : std::nullopt
        );
    }
}
