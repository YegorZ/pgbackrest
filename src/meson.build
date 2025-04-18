####################################################################################################################################
# Make externed functions/variables static when unity is enabled
####################################################################################################################################
arg_unity = []

if get_option('unity') == 'on'
    configuration.set('FN_EXTERN', 'static', description: 'Function is static for unity build')
    configuration.set('VR_EXTERN_DECLARE', 'static', description: 'Variable declaration is static for unity build')
    configuration.set('VR_EXTERN_DEFINE', 'static', description: 'Variable definition is static for unity build')

    # Add args to builds to prevent non-pgbackrest binaries failing due to unused functions
    arg_unity += '-Wno-unused-function'
else
    configuration.set(
        'FN_EXTERN', 'extern __attribute__((visibility ("internal")))',
        description: 'Extern function required by other compilation units')
    configuration.set(
        'VR_EXTERN_DECLARE', 'extern __attribute__((visibility ("internal")))',
        description: 'Extern variable declaration required by other compilation units')
    configuration.set('VR_EXTERN_DEFINE', '', description: 'Extern variable definition required by other compilation units')
endif

####################################################################################################################################
# Write configuration
####################################################################################################################################
configure_file(output: 'build.auto.h', configuration: configuration)

####################################################################################################################################
# Common source used by all targets
####################################################################################################################################
src_common = files(
    'common/debug.c',
    'common/encode.c',
    'common/error/error.c',
    'common/io/filter/buffer.c',
    'common/io/filter/filter.c',
    'common/io/filter/group.c',
    'common/io/filter/sink.c',
    'common/io/bufferRead.c',
    'common/io/bufferWrite.c',
    'common/io/io.c',
    'common/io/read.c',
    'common/io/write.c',
    'common/log.c',
    'common/memContext.c',
    'common/stackTrace.c',
    'common/time.c',
    'common/type/blob.c',
    'common/type/buffer.c',
    'common/type/convert.c',
    'common/type/keyValue.c',
    'common/type/list.c',
    'common/type/object.c',
    'common/type/pack.c',
    'common/type/stringId.c',
    'common/type/stringList.c',
    'common/type/stringStatic.c',
    'common/type/stringZ.c',
    'common/type/variant.c',
    'common/type/variantList.c',
    'common/user.c',
    'common/wait.c',
    'config/common.c',
    'storage/posix/read.c',
    'storage/posix/storage.c',
    'storage/posix/write.c',
    'storage/iterator.c',
    'storage/list.c',
    'storage/read.c',
    'storage/storage.c',
    'storage/write.c',
)

####################################################################################################################################
# Build code target
####################################################################################################################################
src_build_code = [
    'build/common/regExp.c',
    'build/common/render.c',
    'build/common/string.c',
    'build/common/xml.c',
    'build/common/yaml.c',
    'build/config/parse.c',
    'build/config/render.c',
    'build/error/parse.c',
    'build/error/render.c',
    'build/help/parse.c',
    'build/help/render.c',
    'build/postgres/parse.c',
    'build/postgres/render.c',
    'build/main.c',
    'common/compress/common.c',
    'common/compress/bz2/common.c',
    'common/compress/bz2/compress.c',
]

build_code = executable(
    'build-code',
    src_common,
    src_build_code,
    dependencies: [
        lib_backtrace,
        lib_bz2,
        lib_xml,
        lib_yaml
    ],
    c_args: [
        arg_unity,
    ],
)

# Build help.auto.c.inc
subdir('command/help')

# Build interface.auto.c.inc
subdir('postgres')

####################################################################################################################################
# pgBackRest target
####################################################################################################################################
src_pgbackrest = [
    'command/annotate/annotate.c',
    'command/archive/common.c',
    'command/archive/find.c',
    'command/archive/get/file.c',
    'command/archive/get/get.c',
    'command/archive/get/protocol.c',
    'command/archive/push/file.c',
    'command/archive/push/protocol.c',
    'command/archive/push/push.c',
    'command/backup/backup.c',
    'command/backup/blockIncr.c',
    'command/backup/blockMap.c',
    'command/backup/common.c',
    'command/backup/pageChecksum.c',
    'command/backup/protocol.c',
    'command/backup/file.c',
    'command/check/check.c',
    'command/check/common.c',
    'command/check/report.c',
    'command/exit.c',
    'command/expire/expire.c',
    'command/help/help.c',
    'command/info/info.c',
    'command/command.c',
    'command/control/common.c',
    'command/control/start.c',
    'command/control/stop.c',
    'command/local/local.c',
    'command/lock.c',
    'command/manifest/manifest.c',
    'command/repo/common.c',
    'command/repo/get.c',
    'command/repo/ls.c',
    'command/repo/put.c',
    'command/repo/rm.c',
    'command/restore/blockChecksum.c',
    'command/restore/blockDelta.c',
    'command/restore/file.c',
    'command/restore/protocol.c',
    'command/restore/restore.c',
    'command/restore/timeline.c',
    'command/remote/remote.c',
    'command/server/ping.c',
    'command/server/server.c',
    'command/stanza/common.c',
    'command/stanza/create.c',
    'command/stanza/delete.c',
    'command/stanza/upgrade.c',
    'command/verify/file.c',
    'command/verify/protocol.c',
    'command/verify/verify.c',
    'common/compress/common.c',
    'common/compress/helper.c',
    'common/compress/bz2/common.c',
    'common/compress/bz2/compress.c',
    'common/compress/bz2/decompress.c',
    'common/compress/gz/common.c',
    'common/compress/gz/compress.c',
    'common/compress/gz/decompress.c',
    'common/compress/lz4/common.c',
    'common/compress/lz4/compress.c',
    'common/compress/lz4/decompress.c',
    'common/compress/zst/common.c',
    'common/compress/zst/compress.c',
    'common/compress/zst/decompress.c',
    'common/crypto/cipherBlock.c',
    'common/crypto/common.c',
    'common/crypto/hash.c',
    'common/crypto/xxhash.c',
    'common/error/retry.c',
    'common/exec.c',
    'common/fork.c',
    'common/ini.c',
    'common/io/client.c',
    'common/io/fd.c',
    'common/io/fdRead.c',
    'common/io/fdWrite.c',
    'common/io/filter/size.c',
    'common/io/http/client.c',
    'common/io/http/common.c',
    'common/io/http/header.c',
    'common/io/http/query.c',
    'common/io/http/request.c',
    'common/io/http/response.c',
    'common/io/http/session.c',
    'common/io/http/url.c',
    'common/io/limitRead.c',
    'common/io/server.c',
    'common/io/session.c',
    'common/io/socket/address.c',
    'common/io/socket/client.c',
    'common/io/socket/common.c',
    'common/io/socket/server.c',
    'common/io/socket/session.c',
    'common/io/tls/client.c',
    'common/io/tls/common.c',
    'common/io/tls/server.c',
    'common/io/tls/session.c',
    'common/lock.c',
    'common/regExp.c',
    'common/stat.c',
    'common/type/json.c',
    'common/type/string.c',
    'common/type/xml.c',
    'config/config.c',
    'config/exec.c',
    'config/load.c',
    'config/parse.c',
    'config/protocol.c',
    'db/db.c',
    'db/helper.c',
    'db/protocol.c',
    'info/info.c',
    'info/infoArchive.c',
    'info/infoBackup.c',
    'info/manifest.c',
    'info/infoPg.c',
    'postgres/client.c',
    'postgres/interface.c',
    'postgres/interface/crc32.c',
    'postgres/interface/page.c',
    'protocol/client.c',
    'protocol/helper.c',
    'protocol/parallel.c',
    'protocol/parallelJob.c',
    'protocol/server.c',
    'storage/azure/helper.c',
    'storage/azure/read.c',
    'storage/azure/storage.c',
    'storage/azure/write.c',
    'storage/cifs/helper.c',
    'storage/cifs/storage.c',
    'storage/gcs/helper.c',
    'storage/gcs/read.c',
    'storage/gcs/storage.c',
    'storage/gcs/write.c',
    'storage/helper.c',
    'storage/remote/read.c',
    'storage/remote/protocol.c',
    'storage/remote/storage.c',
    'storage/remote/write.c',
    'storage/s3/helper.c',
    'storage/s3/read.c',
    'storage/s3/storage.c',
    'storage/s3/write.c',
    'storage/sftp/helper.c',
    'storage/sftp/read.c',
    'storage/sftp/storage.c',
    'storage/sftp/write.c',
    'main.c',
]

executable(
    'pgbackrest',
    src_common,
    src_pgbackrest,
    help_auto_c_inc,
    interface_auto_c_inc,
    dependencies: [
        lib_backtrace,
        lib_bz2,
        lib_openssl,
        lib_lz4,
        lib_pq,
        lib_ssh2,
        lib_xml,
        lib_z,
        lib_zstd,
    ],
    install: true,
)
