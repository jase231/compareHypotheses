#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriter.hxx>

#include <TRandom.h>

#include <memory>
#include <vector>

constexpr char const* kNTupleFileName = "ntpl002_vector.root";
constexpr int kNEvents = 25000;

std::string build_rntuple()
{
   auto model = ROOT::RNTupleModel::Create();

   auto fldVpx   = model->MakeField<std::vector<float>>("vpx");
   auto fldVpy   = model->MakeField<std::vector<float>>("vpy");
   auto fldVpz   = model->MakeField<std::vector<float>>("vpz");
   auto fldVrand = model->MakeField<std::vector<float>>("vrand");

   auto writer = ROOT::RNTupleWriter::Recreate(std::move(model), "F", kNTupleFileName);

   gRandom->SetSeed();
   for (int i = 0; i < kNEvents; i++) {
      int npx = static_cast<int>(gRandom->Rndm(1) * 15);

      fldVpx->clear();
      fldVpy->clear();
      fldVpz->clear();
      fldVrand->clear();

      for (int j = 0; j < npx; ++j) {
         float px, py, pz;
         gRandom->Rannor(px, py);
         pz = px*px + py*py;

         fldVpx->emplace_back(px);
         fldVpy->emplace_back(py);
         fldVpz->emplace_back(pz);
         fldVrand->emplace_back(gRandom->Rndm(1));
      }

      writer->Fill();
   }
   return std::string(kNTupleFileName);
}
