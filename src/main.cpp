#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "./aids.hpp"

using namespace aids;

struct Target
{
    String_View name;
    Dynamic_Array<String_View> deps;
    String_View script;
};

void print1(FILE *stream, Target target)
{
    println(stream, "Target {");
    println(stream, "    .name = ", target.name);
    print  (stream, "    .deps =");
    for (size_t i = 0; i < target.deps.size; ++i) {
        print(stream, ' ', target.deps.data[i]);
    }
    println(stream);
    println(stream, "    .script = ", target.script);
    println(stream, "}");
}

void parse_targets(String_View content, Dynamic_Array<Target> *output)
{
    bool script = false;
    Target target = {};

    while (content.count > 0) {
        String_View line = content.chop_by_delim('\n');
        if (!script) {
            if (line.count != 0) {
                target.name = line.chop_by_delim(':').trim();
                target.deps = {};
                while (line.count > 0) {
                    target.deps.push(line.chop_word());
                }
                target.script = {0, content.data};
                script = true;
            }
        } else {
            if (line.count == 0) {
                output->push(target);
                script = false;
            } else {
                target.script.grow(line.count + (target.script.count == 0 ? 0 : 1));
            }
        }
    }

    if (script) {
        output->push(target);
    }
}

Maybe<Target> get_target_by_name(Dynamic_Array<Target> targets, String_View name)
{
    for (size_t i = 0; i < targets.size; ++i) {
        if (targets.data[i].name == name) {
            return {true, targets.data[i]};
        }
    }

    return {};
}

bool file_path_exists(String_View path, String_Buffer *temp_buffer)
{
    struct stat statbuf = {};
    temp_buffer->size = 0;
    sprint(temp_buffer, path);
    return stat(temp_buffer->data, &statbuf) == 0;
}

void build_target(String_View target_name,
                  Dynamic_Array<Target> targets,
                  Dynamic_Array<String_View> *visited,
                  String_Buffer *buffer)
{
    visited->push(target_name);

    if (file_path_exists(target_name, buffer)) {
        return;
    }

    auto maybe_target = get_target_by_name(targets, target_name);
    if (!maybe_target.has_value) {
        println(stderr, "Trying to build non-existing target: `", target_name, "`");
        exit(1);
    }
    auto target = maybe_target.unwrap;

    for (size_t i = 0; i < target.deps.size; ++i) {
        if (!visited->contains(target.deps.data[i])) {
            build_target(target.deps.data[i], targets, visited, buffer);
        }
    }

    pid_t pid = fork();
    if (pid) {
        int status = 0;
        waitpid(pid, &status, 0);

        if ((!WIFEXITED(status)) || (WEXITSTATUS(status) != 0)) {
            println(stderr, "Could not build the target: ", target_name);
            exit(1);
        }
    } else {
        println(stdout, target.script);
        buffer->size = 0;
        sprintln(buffer, target.script);
        execlp("sh", "sh", "-c", buffer->data, NULL);
    }
}

int main(int argc, char *argv[])
{
    const char *const cbt_filepath = "./CBT";
    const auto maybe_cbt_content = read_file_as_string_view(cbt_filepath);
    if (!maybe_cbt_content.has_value) {
        println(stderr, "Could not read file `", cbt_filepath, "`: ", strerror(errno));
        exit(1);
    }

    auto cbt_content = maybe_cbt_content.unwrap;
    Dynamic_Array<Target> targets = {};

    parse_targets(cbt_content, &targets);
    if (targets.size > 0) {
        Dynamic_Array<String_View> visited = {};
        String_Buffer script_buffer = {};
        script_buffer.capacity = cbt_content.count;
        script_buffer.data = (char*) malloc(sizeof(char) * script_buffer.capacity);
        build_target(targets.data[0].name, targets, &visited, &script_buffer);
    }

    return 0;
}
