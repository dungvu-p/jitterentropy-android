package eu.npay.android.sdk.softpos.jitterentropy;

/** Java and Kotlin entry point for the Jitterentropy native library. */
public final class JitterEntropy {
    /** Version of the bundled jitterentropy-library C implementation. */
    public static final String VERSION_NAME = BuildConfig.JITTERENTROPY_VERSION_NAME;

    static {
        System.loadLibrary("jitterentropy_android");
    }

    private JitterEntropy() {
    }

    /**
     * Returns {@code length} bytes produced by Jitterentropy.
     *
     * <p>This call performs CPU-intensive entropy collection. Callers should
     * invoke it away from Android's main thread.</p>
     *
     * @param length number of entropy bytes to return; zero is allowed
     * @return a newly allocated byte array containing the entropy
     * @throws IllegalArgumentException if {@code length} is negative
     * @throws IllegalStateException if native initialization or collection fails
     */
    public static byte[] getRandomEntropy(int length) {
        if (length < 0) {
            throw new IllegalArgumentException("length must not be negative");
        }

        byte[] entropy = new byte[length];
        if (length == 0) {
            return entropy;
        }

        int status = nativeGetRandomEntropy(entropy);
        if (status != 0) {
            throw new IllegalStateException(
                    "Jitterentropy failed with native status " + status);
        }
        return entropy;
    }

    private static native int nativeGetRandomEntropy(byte[] output);
}
