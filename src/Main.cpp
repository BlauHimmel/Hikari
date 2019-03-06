#include <core\Screen.hpp>
#include <thread>
#include <mutex>
#include <algorithm>
#include <atomic>

NAMESPACE_BEGIN

static std::mutex Lock;

static void RenderBlock(const Scene * pScene, Sampler * pSampler, ImageBlock & Block)
{
	const Camera * pCamera = pScene->GetCamera();
	const Integrator * pIntegrator = pScene->GetIntegrator();

	Point2i Offset = Block.GetOffset();
	Vector2i Size = Block.GetSize();

	/* Clear the block contents */
	Block.Clear();

	/* For each pixel and pixel sample sample */
	for (int y = 0; y < Size.y(); ++y)
	{
		for (int x = 0; x < Size.x(); ++x)
		{
			for (uint32_t i = 0; i < pSampler->GetSampleCount(); ++i)
			{
				Point2f PixelSample = Point2f(float(x + Offset.x()), float(y + Offset.y())) + pSampler->Next2D();
				Point2f ApertureSample = pSampler->Next2D();

				/* Sample a ray from the camera */
				Ray3f Ray;
				Color3f Value = pCamera->SampleRay(Ray, PixelSample, ApertureSample);

				/* Compute the incident radiance */
				Value *= pIntegrator->Li(pScene, pSampler, Ray);

				/* Store in the image block */
				Block.Put(PixelSample, Value);
			}
		}
	}
}

static void Render(Scene * pScene, const std::string & Filename) 
{
	const Camera * pCamera = pScene->GetCamera();
	Vector2i OutputSize = pCamera->GetOutputSize();
	pScene->GetIntegrator()->Preprocess(pScene);

	/* Create a block generator (i.e. a work scheduler) */
	BlockGenerator BlockGenerator(OutputSize, HIKARI_BLOCK_SIZE);

	int TotalBlock = BlockGenerator.GetBlockCount();
	std::atomic<int> RenderedBlock = 0;

	/* Allocate memory for the entire output image and clear it */
	ImageBlock Result(OutputSize, pCamera->GetReconstructionFilter());
	Result.Clear();

	/* Create a window that visualizes the partially rendered result */
	std::unique_ptr<Screen> pScreen(new Screen(Result));
	std::vector<const ImageBlock *> & RenderingBlocks = pScreen->GetRenderingBlocks();
	float & Progress = pScreen->GetProgress();
	std::string & RenderTimeString = pScreen->GetRenderTimeString();;

	/* Do the following in parallel and asynchronously */
	std::thread RenderThread([&]
	{
		LOG(INFO) << "Rendering ... ";
		Timer RenderTimer;

		tbb::blocked_range<int> Range(0, BlockGenerator.GetBlockCount());

		auto Map = [&](const tbb::blocked_range<int> & Range)
		{
			/* Allocate memory for a small image block to be rendered
			by the current thread */
			ImageBlock Block(Vector2i(HIKARI_BLOCK_SIZE), pCamera->GetReconstructionFilter());

			/* Create a clone of the sampler for the current thread */
			std::unique_ptr<Sampler> pSampler(pScene->GetSampler()->Clone());

			for (int i = Range.begin(); i < Range.end(); ++i)
			{
				/* Request an image block from the block generator */
				BlockGenerator.Next(Block);

				/* Inform the sampler about the block to be rendered */
				pSampler->Prepare(Block);

				/* Add this block to the rendering blocks vector in the Screen class to display it. */
				Lock.lock();
				RenderingBlocks.push_back(&Block);
				Lock.unlock();

				/* Render all contained pixels */
				RenderBlock(pScene, pSampler.get(), Block);

				/* Update progress */
				RenderedBlock++;

				/* Render task is done, remove it. */
				Lock.lock();
				RenderingBlocks.erase(std::remove(RenderingBlocks.begin(), RenderingBlocks.end(), &Block), RenderingBlocks.end());
				Progress = float(RenderedBlock) / float(TotalBlock);
				RenderTimeString = RenderTimer.ElapsedString();
				Lock.unlock();

				/* The image block has been processed. Now add it to
				the "big" block that represents the entire image */
				Result.Put(Block);
			}
		};

		/// Uncomment the following line for single threaded rendering
		//Map(Range);

		/// Default: parallel rendering
		tbb::parallel_for(Range, Map);

		LOG(INFO) << "Done. (took " << RenderTimer.ElapsedString() << ")";
	});

	/* Enter the application main loop */
	pScreen->Draw();

	/* Shut down the user interface */
	RenderThread.join();

	/* Now turn the rendered image block into
	a properly normalized bitmap */
	std::unique_ptr<Bitmap> pBitmap(Result.ToBitmap());

	/* Determine the filename of the output bitmap */
	std::string OutputName = Filename;
	size_t iLastDot = OutputName.find_last_of(".");
	if (iLastDot != std::string::npos)
	{
		OutputName.erase(iLastDot, std::string::npos);
	}
	OutputName += ".exr";

	/* Save using the OpenEXR format */
	pBitmap->Save(OutputName);
}

NAMESPACE_END

int main(int argc, char ** argv)
{
	google::InitGoogleLogging("Hikari");
	google::SetStderrLogging(google::GLOG_INFO);

	if (argc != 2)
	{
		LOG(ERROR) << "Syntax: " << argv[0] << " <scene.xml> or <image.exr>";
		return -1;
	}

	filesystem::path Path(argv[1]);

	try
	{
		if (Path.extension() == "xml")
		{
			/* Add the parent directory of the scene file to the
			file resolver. That way, the XML file can reference
			resources (OBJ files, textures) using relative paths */
			Hikari::GetFileResolver()->prepend(Path.parent_path());

			std::unique_ptr<Hikari::Object> Root(Hikari::LoadFromXML(argv[1]));

			/* When the XML root object is a scene, start rendering it .. */
			if (Root->GetClassType() == Hikari::Object::EClassType::EScene)
			{
				Hikari::Render((Hikari::Scene*)(Root.get()), argv[1]);
			}
		}
		else if (Path.extension() == "exr")
		{
			/* Alternatively, provide a basic OpenEXR image viewer */
			Hikari::Bitmap Bit(argv[1]);
			Hikari::ImageBlock Block(Hikari::Vector2i(int(Bit.cols()), int(Bit.rows())), nullptr);
			Block.FromBitmap(Bit);
			std::unique_ptr<Hikari::Screen> pScreen(new Hikari::Screen(Block));
			pScreen->Draw();
		}
		else
		{
			LOG(ERROR) << "Fatal error: unknown file \"" << argv[1] << "\", expected an extension of type .xml or .exr";
		}
	}
	catch (const std::exception & Ex)
	{
		LOG(ERROR) << "Fatal error: " << Ex.what();
	}

	google::ShutdownGoogleLogging();
	system("PAUSE");
	return 0;
}

