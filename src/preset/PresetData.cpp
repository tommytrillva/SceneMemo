#include "preset/PresetData.h"

namespace scenememo {

juce::String PresetData::toJSON() const
{
    auto root = std::make_unique<juce::DynamicObject>();

    // Metadata
    auto meta = std::make_unique<juce::DynamicObject>();
    meta->setProperty ("name", metadata.name);
    meta->setProperty ("author", metadata.author);
    meta->setProperty ("category", static_cast<int> (metadata.category));
    meta->setProperty ("description", metadata.description);
    meta->setProperty ("sourceAudioPath", metadata.sourceAudioPath);
    meta->setProperty ("locationTag", metadata.locationTag);
    meta->setProperty ("dateCreated", metadata.dateCreated);
    meta->setProperty ("dateModified", metadata.dateModified);
    meta->setProperty ("isFavorite", metadata.isFavorite);

    juce::Array<juce::var> tagArray;
    for (const auto& tag : metadata.tags)
        tagArray.add (tag);
    meta->setProperty ("tags", tagArray);

    root->setProperty ("metadata", meta.release());

    // Parameter state as XML string
    if (parameterState.isValid())
    {
        auto xml = parameterState.createXml();
        if (xml != nullptr)
            root->setProperty ("parameters", xml->toString());
    }

    root->setProperty ("version", "1.0");

    return juce::JSON::toString (juce::var (root.release()));
}

PresetData PresetData::fromJSON (const juce::String& jsonString)
{
    PresetData preset;

    auto parsed = juce::JSON::parse (jsonString);
    if (! parsed.isObject())
        return preset;

    auto* root = parsed.getDynamicObject();
    if (root == nullptr)
        return preset;

    // Parse metadata
    auto metaVar = root->getProperty ("metadata");
    if (auto* meta = metaVar.getDynamicObject())
    {
        preset.metadata.name            = meta->getProperty ("name").toString();
        preset.metadata.author          = meta->getProperty ("author").toString();
        preset.metadata.category        = static_cast<PresetCategory> (static_cast<int> (meta->getProperty ("category")));
        preset.metadata.description     = meta->getProperty ("description").toString();
        preset.metadata.sourceAudioPath = meta->getProperty ("sourceAudioPath").toString();
        preset.metadata.locationTag     = meta->getProperty ("locationTag").toString();
        preset.metadata.dateCreated     = meta->getProperty ("dateCreated").toString();
        preset.metadata.dateModified    = meta->getProperty ("dateModified").toString();
        preset.metadata.isFavorite      = meta->getProperty ("isFavorite");

        if (auto* tags = meta->getProperty ("tags").getArray())
        {
            for (const auto& tag : *tags)
                preset.metadata.tags.add (tag.toString());
        }
    }

    // Parse parameters
    auto paramXmlStr = root->getProperty ("parameters").toString();
    if (paramXmlStr.isNotEmpty())
    {
        auto xml = juce::XmlDocument::parse (paramXmlStr);
        if (xml != nullptr)
            preset.parameterState = juce::ValueTree::fromXml (*xml);
    }

    return preset;
}

bool PresetData::saveToFile (const juce::File& file) const
{
    return file.replaceWithText (toJSON());
}

PresetData PresetData::loadFromFile (const juce::File& file)
{
    if (! file.existsAsFile())
        return {};

    return fromJSON (file.loadFileAsString());
}

} // namespace scenememo
