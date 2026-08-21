#include <jni.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

#include "jitterentropy.h"

#define ENTROPY_CHUNK_SIZE 256
#define JENT_WRAPPER_THREAD_ERROR 1000
#define JENT_WRAPPER_ALLOCATION_ERROR EMEM

static pthread_once_t initialization_once = PTHREAD_ONCE_INIT;
static pthread_mutex_t collector_mutex = PTHREAD_MUTEX_INITIALIZER;
static int initialization_status = EPROGERR;
static struct rand_data *collector = NULL;

static void initialize_jitterentropy(void)
{
    initialization_status = jent_entropy_init();
    if (initialization_status != 0)
        return;

    collector = jent_entropy_collector_alloc(0, 0);
    if (collector == NULL)
        initialization_status = JENT_WRAPPER_ALLOCATION_ERROR;
}

static void secure_clear(void *buffer, size_t length)
{
    volatile unsigned char *current = (volatile unsigned char *)buffer;

    while (length-- > 0)
        *current++ = 0;
}

static void clear_java_array(JNIEnv *env, jbyteArray output, jsize length)
{
    jbyte zeros[ENTROPY_CHUNK_SIZE] = { 0 };
    jsize offset = 0;

    while (offset < length) {
        jsize remaining = length - offset;
        jsize chunk = remaining < ENTROPY_CHUNK_SIZE
                      ? remaining : ENTROPY_CHUNK_SIZE;
        (*env)->SetByteArrayRegion(env, output, offset, chunk, zeros);
        if ((*env)->ExceptionCheck(env))
            return;
        offset += chunk;
    }
}

JNIEXPORT jint JNICALL
Java_eu_npay_android_sdk_softpos_jitterentropy_JitterEntropy_nativeGetRandomEntropy(
    JNIEnv *env, jclass clazz, jbyteArray output)
{
    char entropy[ENTROPY_CHUNK_SIZE];
    jsize length;
    jsize offset = 0;
    int once_status;
    jint status = 0;

    (void)clazz;

    if (output == NULL)
        return JENT_ERR_EINVAL;

    length = (*env)->GetArrayLength(env, output);
    if (length == 0)
        return 0;

    once_status = pthread_once(&initialization_once, initialize_jitterentropy);
    if (once_status != 0)
        return JENT_WRAPPER_THREAD_ERROR + once_status;
    if (initialization_status != 0)
        return initialization_status;

    once_status = pthread_mutex_lock(&collector_mutex);
    if (once_status != 0)
        return JENT_WRAPPER_THREAD_ERROR + once_status;

    while (offset < length) {
        jsize remaining = length - offset;
        jsize chunk = remaining < ENTROPY_CHUNK_SIZE
                      ? remaining : ENTROPY_CHUNK_SIZE;
        ssize_t result = jent_read_entropy_safe(
            &collector, entropy, (size_t)chunk);

        if (result != chunk) {
            status = result < 0 ? (jint)result : JENT_ERR_EINVAL;
            clear_java_array(env, output, length);
            break;
        }

        (*env)->SetByteArrayRegion(
            env, output, offset, chunk, (const jbyte *)entropy);
        if ((*env)->ExceptionCheck(env))
            break;

        secure_clear(entropy, (size_t)chunk);
        offset += chunk;
    }

    secure_clear(entropy, sizeof(entropy));
    once_status = pthread_mutex_unlock(&collector_mutex);
    if (status == 0 && once_status != 0)
        status = JENT_WRAPPER_THREAD_ERROR + once_status;
    return status;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
    (void)vm;
    (void)reserved;

    if (pthread_mutex_lock(&collector_mutex) != 0)
        return;
    if (collector != NULL) {
        jent_entropy_collector_free(collector);
        collector = NULL;
    }
    (void)pthread_mutex_unlock(&collector_mutex);
}
