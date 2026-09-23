#include <NanoTest/NanoTest.h>
#include <ea_data_structures/Structures/Immutable.h>
#include <stdexcept>
#include <string>
#include <string_view>

using namespace nano;
using namespace EA;

namespace
{
struct Clip
{
    std::string file;
    double start = 0.0;
};

struct TrackNode
{
    std::string name;
    float gain = 1.f;
    bool muted = false;
    ImmutableVector<Clip> clips;
    ImmutableVector<TrackNode> children; //Recursive: TrackNode is incomplete here
};

struct Settings
{
    double tempo = 120.0;
};

struct Project
{
    TrackNode rootTrack; //Plain inline member
    Immutable<Settings> settings; //Shared subtree
};

void muteByName(const TrackNode& node, std::string_view name)
{
    auto shouldMute = node.name == name;

    if (node.muted != shouldMute)
        write(node).muted = shouldMute;

    forEach(node.children, [&](const TrackNode& child) { muteByName(child, name); });
}

TrackNode makeTrack(const std::string& name,
                    std::initializer_list<TrackNode> children = {})
{
    auto track = TrackNode();
    track.name = name;
    track.clips.add(Clip {name + ".wav", 0.0});

    for (auto& child: children)
        track.children.add(child);

    return track;
}

//Master [Drums, Bass, Group [Kick, Snare]]
Immutable<Project> makeProject()
{
    auto project = Project();
    project.rootTrack =
        makeTrack("Master",
                  {makeTrack("Drums"),
                   makeTrack("Bass"),
                   makeTrack("Group", {makeTrack("Kick"), makeTrack("Snare")})});
    project.settings = Settings {140.0};

    return project;
}

template <typename Fn>
bool throwsPrecondition(Fn&& fn)
{
    try
    {
        fn();
    }
    catch (const ImmutablePreconditionError&)
    {
        return true;
    }

    return false;
}

//Counts copies (and live instances) to check that nodes are cloned only once
struct Counted
{
    Counted() { ++alive; }
    Counted(const Counted&)
    {
        ++copies;
        ++alive;
    }
    Counted& operator=(const Counted&) = default;
    ~Counted() { --alive; }

    static inline int copies = 0;
    static inline int alive = 0;
};

struct CountedNode
{
    Counted counter;
    int value = 0;
    ImmutableVector<CountedNode> children;
};

CountedNode makeCounted(int value, int numChildren = 0)
{
    auto node = CountedNode();
    node.value = value;

    for (int index = 0; index < numChildren; ++index)
        node.children.add(makeCounted(value * 10 + index));

    return node;
}
} // namespace

auto immutableConstructs = test("Immutable.constructs_and_reads") = []
{
    auto settings = Immutable<Settings>(Settings {90.0});
    check(settings->tempo == 90.0);
    check((*settings).tempo == 90.0);
    check(settings.get().tempo == 90.0);

    auto inPlace = Immutable<std::string>(std::in_place, 3, 'x');
    check(*inPlace == "xxx");
};

auto immutableCopyShares = test("Immutable.copy_shares_identity") = []
{
    auto a = Immutable<Settings>(Settings {100.0});
    auto b = a;
    check(a.isSameAs(b));
    check(&a.get() == &b.get());

    auto c = Immutable<Settings>(Settings {100.0});
    check(!a.isSameAs(c));

    auto moved = std::move(b);
    check(moved.isSameAs(a));
};

auto immutableDefault = test("Immutable.default_state_reads_shared_default") = []
{
    auto a = Immutable<Settings>();
    auto b = Immutable<Settings>();
    check(a->tempo == 120.0);
    check(a.isSameAs(b));
    check(&a.get() == &b.get());

    //Recursive type: default construction doesn't allocate or recurse
    auto track = Immutable<TrackNode>();
    check(track->children.isEmpty());
    check(track->name.empty());
};

auto immutableUpdateDefault = test("Immutable.update_default_root") = []
{
    auto settings = Immutable<Settings>();
    settings.update([](const Settings& s) { write(s).tempo = 60.0; });
    check(settings->tempo == 60.0);
    check(Immutable<Settings>()->tempo == 120.0);
};

auto immutableVectorReads = test("ImmutableVector.const_access_and_iteration") = []
{
    auto clips = ImmutableVector<Clip> {Clip {"a.wav", 0.0}, Clip {"b.wav", 1.0}};
    check(clips.size() == 2);
    check(!clips.isEmpty());
    check(clips[1].file == "b.wav");
    check(clips.front().file == "a.wav");
    check(clips.back().start == 1.0);

    auto names = std::string();

    for (auto& clip: clips)
        names += clip.file;

    check(names == "a.wavb.wav");

    auto copy = clips;
    check(copy.isSameAs(clips));
    check(copy.isSameAs(0, clips));
};

auto immutableMuteByName = test("Immutable.path_copy_only_changed_branch") = []
{
    auto project = makeProject();
    auto before = project;

    project.update([&](const Project& p) { muteByName(p.rootTrack, "Snare"); });

    check(!project.isSameAs(before));

    auto& root = project->rootTrack;
    auto& oldRoot = before->rootTrack;

    //Untouched: siblings, clips, settings
    check(root.children.isSameAs(0, oldRoot.children));
    check(root.children.isSameAs(1, oldRoot.children));
    check(root.clips.isSameAs(oldRoot.clips));
    check(project->settings.isSameAs(before->settings));

    //Changed branch: Group and Snare are new, Kick is shared
    check(!root.children.isSameAs(2, oldRoot.children));
    auto& group = root.children[2];
    auto& oldGroup = oldRoot.children[2];
    check(group.children.isSameAs(0, oldGroup.children));
    check(!group.children.isSameAs(1, oldGroup.children));
    check(group.clips.isSameAs(oldGroup.clips));

    check(group.children[1].muted);
    check(!oldGroup.children[1].muted);
    check(!group.children[0].muted);
    check(!root.muted);
};

auto immutableMuteDirectChild = test("Immutable.path_copy_direct_child") = []
{
    auto project = makeProject();
    auto before = project;

    project.update([&](const Project& p) { muteByName(p.rootTrack, "Drums"); });

    auto& root = project->rootTrack;
    auto& oldRoot = before->rootTrack;

    check(!root.children.isSameAs(0, oldRoot.children));
    check(root.children.isSameAs(1, oldRoot.children));
    check(root.children.isSameAs(2, oldRoot.children));
    check(root.children[0].clips.isSameAs(oldRoot.children[0].clips));
    check(project->settings.isSameAs(before->settings));
    check(root.children[0].muted);
    check(!oldRoot.children[0].muted);
};

auto immutableNoOp = test("Immutable.noop_update_keeps_identity") = []
{
    auto project = makeProject();
    auto before = project;

    project.update([&](const Project& p) { muteByName(p.rootTrack, "Nothing"); });

    check(project.isSameAs(before));
};

auto immutableForEachIndex = test("Immutable.forEach_with_index") = []
{
    auto project = makeProject();

    project.update(
        [&](const Project& p)
        {
            forEach(p.rootTrack.children,
                    [&](const TrackNode& child, int index)
                    {
                        if (index == 1)
                            write(child).gain = 0.25f;
                    });
        });

    check(project->rootTrack.children[1].gain == 0.25f);
    check(project->rootTrack.children[0].gain == 1.f);
};

auto immutableCopiedOnce = test("Immutable.node_copied_once_per_edit") = []
{
    auto root = Immutable<CountedNode>(makeCounted(1, 2));
    auto before = root;
    Counted::copies = 0;

    root.update(
        [&](const CountedNode& node)
        {
            write(node).value = 2;
            write(node).value += 1;

            forEach(node.children,
                    [&](const CountedNode& child, int index)
                    {
                        if (index == 0)
                        {
                            write(child).value = 100;
                            write(child).value += 1;
                        }
                    });

            //Second visit of the same child reuses its draft
            forEach(node.children,
                    [&](const CountedNode& child, int index)
                    {
                        if (index == 0)
                            write(child).value += 1;
                    });

            //So does an eager edit through the draft vector
            write(node).children[0].value += 1;
        });

    check(Counted::copies == 2);
    check(root->value == 3);
    check(root->children[0].value == 103);
    check(root.get().children.isSameAs(1, before->children));
    check(before->children[0].value == 10);
};

auto immutableRecopyNextEdit = test("Immutable.second_edit_copies_again") = []
{
    auto root = Immutable<CountedNode>(makeCounted(1, 2));

    auto touchFirstChild = [&]
    {
        root.update(
            [&](const CountedNode& node)
            {
                forEach(node.children,
                        [&](const CountedNode& child, int index)
                        {
                            if (index == 0)
                                write(child).value += 1;
                        });
            });
    };

    Counted::copies = 0;
    touchFirstChild();
    check(Counted::copies == 2);

    auto afterFirst = root;
    touchFirstChild();

    //Nodes drafted in the first edit are published now, so they're cloned again
    check(Counted::copies == 4);
    check(!root.isSameAs(afterFirst));
    check(!root->children.isSameAs(0, afterFirst->children));
    check(root->children.isSameAs(1, afterFirst->children));
    check(root->children[0].value == 12);
    check(afterFirst->children[0].value == 11);
};

auto immutableEagerEdit = test("Immutable.eager_element_edit") = []
{
    auto project = makeProject();
    auto before = project;

    project.update([&](const Project& p)
                   { write(p.rootTrack).children[1].gain = 0.5f; });

    auto& root = project->rootTrack;
    auto& oldRoot = before->rootTrack;
    check(root.children[1].gain == 0.5f);
    check(oldRoot.children[1].gain == 1.f);
    check(!root.children.isSameAs(1, oldRoot.children));
    check(root.children.isSameAs(0, oldRoot.children));
    check(root.children.isSameAs(2, oldRoot.children));
    check(root.children[1].clips.isSameAs(oldRoot.children[1].clips));
};

auto immutableVisit = test("Immutable.visit_shared_subtree") = []
{
    auto project = makeProject();
    auto before = project;

    project.update(
        [&](const Project& p)
        { visit(p.settings, [](const Settings& s) { write(s).tempo = 90.0; }); });

    check(project->settings->tempo == 90.0);
    check(before->settings->tempo == 140.0);
    check(!project->settings.isSameAs(before->settings));
    check(project->rootTrack.children.isSameAs(before->rootTrack.children));
    check(project->rootTrack.clips.isSameAs(before->rootTrack.clips));
};

auto immutableStructural = test("Immutable.structural_edit") = []
{
    auto project = makeProject();
    auto before = project;

    project.update(
        [&](const Project& p)
        {
            auto& root = write(p.rootTrack);
            root.children.removeAt(0);
            root.children.add(makeTrack("Keys")).gain = 0.5f;
        });

    auto& children = project->rootTrack.children;
    check(children.size() == 3);
    check(children[0].name == "Bass");
    check(children[2].name == "Keys");
    check(children[2].gain == 0.5f);
    check(children.getHandle(0).isSameAs(before->rootTrack.children.getHandle(1)));
    check(before->rootTrack.children.size() == 3);
    check(before->rootTrack.children[0].name == "Drums");
};

auto immutableRollback = test("Immutable.exception_rolls_back") = []
{
    {
        auto root = Immutable<CountedNode>(makeCounted(1, 2));
        auto before = root;
        auto aliveBefore = Counted::alive;

        auto threw = false;

        try
        {
            root.update(
                [&](const CountedNode& node)
                {
                    write(node).value = 5;
                    forEach(node.children,
                            [&](const CountedNode& child)
                            { write(child).value = 7; });
                    throw std::runtime_error("abort");
                });
        }
        catch (const std::runtime_error&)
        {
            threw = true;
        }

        check(threw);
        check(root.isSameAs(before));
        check(root->value == 1);
        check(root->children[0].value == 10);

        //Drafts were released
        check(Counted::alive == aliveBefore);
    }

    check(Counted::alive == 0);
};

auto immutableReleasesOld = test("Immutable.releases_replaced_nodes") = []
{
    {
        auto root = Immutable<CountedNode>(makeCounted(1, 2));
        auto aliveBefore = Counted::alive;

        root.update(
            [&](const CountedNode& node)
            {
                forEach(node.children,
                        [&](const CountedNode& child) { write(child).value = 7; });
            });

        //Nobody else held the old tree, so it was replaced one for one
        check(Counted::alive == aliveBefore);
        check(root->children[1].value == 7);
    }

    check(Counted::alive == 0);
};

auto immutableNested = test("Immutable.nested_updates_of_different_roots") = []
{
    auto a = Immutable<Settings>(Settings {1.0});
    auto b = Immutable<Settings>(Settings {2.0});

    a.update(
        [&](const Settings& sa)
        {
            write(sa).tempo = 10.0;

            b.update(
                [&](const Settings& sb)
                {
                    write(sb).tempo = 20.0;

                    //Found by address even though b's frame is on top
                    write(sa).tempo += 1.0;
                });
        });

    check(a->tempo == 11.0);
    check(b->tempo == 20.0);
};

auto immutableWriteOutside = test("Immutable.write_outside_edit_asserts") = []
{
    auto settings = Immutable<Settings>(Settings {1.0});
    check(throwsPrecondition([&] { write(*settings).tempo = 3.0; }));

    auto project = makeProject();
    check(throwsPrecondition(
        [&] { forEach(project->rootTrack.children, [](const TrackNode&) {}); }));
    check(throwsPrecondition([&]
                             { visit(project->settings, [](const Settings&) {}); }));
    check(project->settings->tempo == 140.0);
};

auto immutableWriteUnrelated =
    test("Immutable.write_on_unrelated_object_asserts") = []
{
    auto project = makeProject();
    auto other = makeProject();
    auto before = project;

    //Nodes reached without forEach/visit have no frame
    check(throwsPrecondition(
        [&]
        {
            project.update([&](const Project&)
                           { write(other->rootTrack.children[0]).gain = 0.f; });
        }));

    check(project.isSameAs(before));
};

auto immutableIterationGuard =
    test("Immutable.structural_change_during_forEach_asserts") = []
{
    auto project = makeProject();
    auto before = project;

    check(throwsPrecondition(
        [&]
        {
            project.update(
                [&](const Project& p)
                {
                    forEach(p.rootTrack.children,
                            [&](const TrackNode&)
                            { write(p.rootTrack).children.add(makeTrack("X")); });
                });
        }));

    check(project.isSameAs(before));
};

auto immutableStaleIndex =
    test("Immutable.stale_index_after_structural_change_asserts") = []
{
    auto project = makeProject();

    check(throwsPrecondition(
        [&]
        {
            project.update(
                [&](const Project& p)
                {
                    write(p.rootTrack).children.removeAt(0);

                    //Iterates the original: index 0 is Drums, which is gone
                    forEach(p.rootTrack.children,
                            [&](const TrackNode& child)
                            { write(child).gain = 0.f; });
                });
        }));
};

auto immutableSameRootNested =
    test("Immutable.nested_update_of_same_root_asserts") = []
{
    auto settings = Immutable<Settings>(Settings {1.0});

    check(throwsPrecondition(
        [&]
        {
            settings.update([&](const Settings&)
                            { settings.update([](const Settings&) {}); });
        }));

    check(settings->tempo == 1.0);
};
