#include <iostream>
#include <vector>

#include <cbmpc/crypto/ro.h>
#include <cbmpc/crypto/secret_sharing.h>
#include <cbmpc/crypto/tdh2.h>

using namespace coinbase::crypto;

// Generate additive shares for TDH2.
static void generate_additive_shares(int n, tdh2::public_key_t& enc_key, tdh2::pub_shares_t& pub_shares,
                                     std::vector<tdh2::private_share_t>& dec_shares, ecurve_t curve) {
  const ecc_point_t& G = curve.generator();

  bn_t x = curve.get_random_value();

  std::vector<bn_t> prv_shares = ss::share_and(curve.order(), x, n);
  pub_shares.resize(n);
  for (int i = 0; i < n; i++) {
    pub_shares[i] = prv_shares[i] * G;
  }
  enc_key.Q = x * G;
  enc_key.Gamma = ro::hash_curve(mem_t("TDH2-Gamma"), enc_key.Q).curve(curve);

  dec_shares.resize(n);
  for (int i = 0; i < n; i++) {
    dec_shares[i].x = prv_shares[i];
    dec_shares[i].pid = i + 1;
    dec_shares[i].pub_key = enc_key;
  }
}

int main() {
  const int n = 3;  // number of nodes
  ecurve_t curve = curve_p256;

  // ==== Decentralized key generation ====
  tdh2::public_key_t enc_key;
  tdh2::pub_shares_t pub_shares;
  std::vector<tdh2::private_share_t> dec_shares;
  generate_additive_shares(n, enc_key, pub_shares, dec_shares, curve);
  std::cout << "Public key generated.\n";

  // ==== Threshold encryption ====
  buf_t plain = buf_t("Hello TDH2");
  buf_t label = buf_t("demo");
  tdh2::ciphertext_t ciphertext = enc_key.encrypt(plain, label);
  if (ciphertext.verify(enc_key, label) != SUCCESS) {
    std::cout << "Ciphertext verification failed\n";
    return 1;
  }
  std::cout << "Ciphertext created and verified.\n";

  // ==== Partial decryptions by each node ====
  tdh2::partial_decryptions_t partials(n);
  for (int i = 0; i < n; i++) {
    dec_shares[i].decrypt(ciphertext, label, partials[i]);
    std::cout << "Node " << (i + 1) << " computed partial decryption.\n";
  }

  // ==== Combine partial decryptions ====
  buf_t decrypted;
  if (tdh2::combine_additive(enc_key, pub_shares, label, partials, ciphertext, decrypted) != SUCCESS) {
    std::cout << "Combine failed\n";
    return 1;
  }
  std::cout << "Decrypted message: " << std::string(reinterpret_cast<char*>(decrypted.data()), decrypted.size())
            << "\n";

  return 0;
}
