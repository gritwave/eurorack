import numpy as np


def rectangle_dither(x):
    return x + np.random.uniform(low=-0.5, high=0.5, size=x.shape)


class TriangleDither:
    def __init__(self):
        self.last = 0

    def __call__(self, v):
        r = np.random.uniform(-0.5, 0.5)
        result = v + r - self.last
        self.last = r
        return result


def snr(signal, dithered_signal):
    # Calculate the power of the original signal
    original_power = np.mean(signal**2)

    # Calculate the power of the difference between the original and dithered signals
    noise_power = np.mean((signal - dithered_signal)**2)

    # Calculate the SNR in dB
    snr = 10 * np.log10(original_power / noise_power)

    return snr


def mse(signal, reconstructed):
    return np.mean((signal - reconstructed) ** 2)


def psnr(signal, reconstructed):
    return 10 * np.log10((np.max(signal) ** 2) / mse(signal, reconstructed))


def thd(signal):
    # Perform FFT on the signal
    fft_result = np.fft.fft(signal)

    # Find the fundamental frequency component
    fundamental_index = np.argmax(np.abs(fft_result))

    # Set the fundamental frequency component to zero
    fft_result[fundamental_index] = 0

    # Reconstruct the signal after removing the fundamental frequency component
    reconstructed_signal = np.fft.ifft(fft_result)

    # Calculate the THD as the ratio of the RMS of the harmonic components to the RMS of the reconstructed signal
    harmonic_rms = np.sqrt(np.mean(np.abs(reconstructed_signal)**2))
    thd = harmonic_rms / np.sqrt(np.mean(np.abs(signal)**2))

    return thd


def to_decibels(x):
    return np.log10(x)*20.0


def thd_diff(signal, reconstructed):
    return to_decibels(np.abs(thd(signal) - thd(reconstructed)))

# Example usage
# Assume signal and dithered_signal are arrays of audio samples


bit_depth = 24
scale = 2**(bit_depth-1)

signal = np.sin(np.linspace(-np.pi, np.pi, 1024, dtype=np.float64)) * 0.5
signal += np.random.uniform(-1.0, 1.0, signal.shape[0]) * 0.125

unrounded_compressed = (signal * scale).astype(np.int32)
unrounded_signal = unrounded_compressed.astype(np.float64) / scale

rounded_compressed = (signal * scale).round().astype(np.int32)
rounded_signal = rounded_compressed.astype(np.float64) / scale

td = TriangleDither()
triangle_compressed = np.vectorize(td)(signal*scale).round().astype(np.int32)
triangle_signal = triangle_compressed.astype(np.float64) / scale


rectangle_compressed = rectangle_dither(signal*scale).round().astype(np.int32)
rectangle_signal = rectangle_compressed.astype(np.float64) / scale

# Calculate SNR
print(f"{snr(signal, unrounded_signal)=}")
print(f"{snr(signal, rounded_signal)=}")
print(f"{snr(signal, rectangle_signal)=}")
print(f"{snr(signal, triangle_signal)=}")
print("")

# Calculate PSNR
print(f"{psnr(signal, unrounded_signal)=}")
print(f"{psnr(signal, rounded_signal)=}")
print(f"{psnr(signal, rectangle_signal)=}")
print(f"{psnr(signal, triangle_signal)=}")
print("")

# # Calculate MSE
# print(f"{mse(signal, unrounded_signal)=}")
# print(f"{mse(signal, rounded_signal)=}")
# print(f"{mse(signal, rectangle_signal)=}")
# print(f"{mse(signal, triangle_signal)=}")
# print("")

# Calculate THD
print(f"{thd_diff(signal, unrounded_signal)=:.2f}")
print(f"{thd_diff(signal, rounded_signal)=:.2f}")
print(f"{thd_diff(signal, rectangle_signal)=:.2f}")
print(f"{thd_diff(signal, triangle_signal)=:.2f}")
print("")
