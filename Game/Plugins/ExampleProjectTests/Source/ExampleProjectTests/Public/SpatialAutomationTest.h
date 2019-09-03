//#include "Misc/AutomationTest.h"
//
//class FSpatialAutomationTestFramework : public FAutomationTestFramework
//{
//public:
//	///** Does any init work across all tests.. */
//	//UFUNCTION(BlueprintCallable, Category = Perf)
//	//	void OnBeginTests();
//	///** Does any final work needed as all tests are complete. */
//	//UFUNCTION(BlueprintCallable, Category = Perf)
//	//	void OnAllTestsComplete();
//
//	/** Constructor */
//	FSpatialAutomationTestFramework()
//	{}
//
//	/** Destructor */
//	~FSpatialAutomationTestFramework();
//
//	/**
//	 * Return the singleton instance of the framework.
//	 *
//	 * @return The singleton instance of the framework.
//	 */
//	static FSpatialAutomationTestFramework& Get();
//	static FSpatialAutomationTestFramework& GetInstance() { return Get(); }
//
//};
//
///** Simple abstract base class for all automation tests */
//class FSpatialAutomationTestBase : public FAutomationTestBase
//{
//public:
//	/**
//	 * Constructor
//	 *
//	 * @param	InName	Name of the test
//	 */
//	FSpatialAutomationTestBase(const FString& InName, const bool bInComplexTask)
//		// TODO probably don't want this I assume, but maybe we don't need a different framework and can add befores and afters?
//		: FAutomationTestBase(InName, bInComplexTask)
//		, bIsDiscoveryMode(false)
//		, bBaseRunTestRan(false)
//	{
//		// Register the newly created automation test into the automation testing framework
//		FSpatialAutomationTestFramework::Get().RegisterAutomationTest(InName, this);
//	}
//
//	/** Destructor */
//	virtual ~FSpatialAutomationTestBase()
//	{
//		// Unregister the automation test from the automation testing framework
//		FSpatialAutomationTestFramework::Get().UnregisterAutomationTest(TestName);
//	}
//
//	///**
//	// * Enqueues a new latent command.
//	// */
//	//FORCEINLINE void AddCommand(IAutomationLatentCommand* NewCommand)
//	//{
//	//	TSharedRef<IAutomationLatentCommand> CommandPtr = MakeShareable(NewCommand);
//	//	FSpatialAutomationTestBase::Get().EnqueueLatentCommand(CommandPtr);
//	//}
//
//	///**
//	// * Enqueues a new latent network command.
//	// */
//	//FORCEINLINE void AddCommand(IAutomationNetworkCommand* NewCommand)
//	//{
//	//	TSharedRef<IAutomationNetworkCommand> CommandPtr = MakeShareable(NewCommand);
//	//	FSpatialAutomationTestBase::Get().EnqueueNetworkCommand(CommandPtr);
//	//}
//
//	virtual bool RunTest(const FString& Parameters) override
//	{
//		bBaseRunTestRan = true;
//		TestIdToExecute = Parameters;
//
//		return true;
//	}
//
//	virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray <FString>& OutTestCommands) const override
//	{
//		const_cast<FSpatialAutomationTestBase*>(this)->BeautifiedNames.Empty();
//		const_cast<FSpatialAutomationTestBase*>(this)->TestCommands.Empty();
//
//		bIsDiscoveryMode = true;
//		const_cast<FSpatialAutomationTestBase*>(this)->RunTest(FString());
//		bIsDiscoveryMode = false;
//
//		OutBeautifiedNames.Append(BeautifiedNames);
//		OutTestCommands.Append(TestCommands);
//		bBaseRunTestRan = false;
//	}
//
//	bool IsDiscoveryMode() const
//	{
//		return bIsDiscoveryMode;
//	}
//
//	void BeforeEach(TFunction<void()> DoWork)
//	{
//		BeforeEachStack.Push(DoWork);
//	}
//
//	void AfterEach(TFunction<void()> DoWork)
//	{
//		AfterEachStack.Push(DoWork);
//	}
//
//private:
//
//	FString TestIdToExecute;
//	TArray<FString> Description;
//	TArray<TFunction<void()>> BeforeEachStack;
//	TArray<TFunction<void()>> AfterEachStack;
//
//	TArray<FString> BeautifiedNames;
//	TArray<FString> TestCommands;
//	mutable bool bIsDiscoveryMode;
//	mutable bool bBaseRunTestRan;
//
//protected:
//	friend class FSpatialAutomationTestFramework;
//};
//
//
//#define IMPLEMENT_SIMPLE_SPATIAL_AUTOMATION_TEST_PRIVATE( TClass, PrettyName, TFlags, FileName, LineNumber ) \
//	class TClass : public FSpatialAutomationTestBase \
//	{ \
//	public: \
//		TClass( const FString& InName ) \
//		:FSpatialAutomationTestBase( InName, false ) {\
//			static_assert((TFlags)&EAutomationTestFlags::ApplicationContextMask, "AutomationTest has no application flag.  It shouldn't run.  See AutomationTest.h."); \
//			static_assert(	(((TFlags)&EAutomationTestFlags::FilterMask) == EAutomationTestFlags::SmokeFilter) || \
//							(((TFlags)&EAutomationTestFlags::FilterMask) == EAutomationTestFlags::EngineFilter) || \
//							(((TFlags)&EAutomationTestFlags::FilterMask) == EAutomationTestFlags::ProductFilter) || \
//							(((TFlags)&EAutomationTestFlags::FilterMask) == EAutomationTestFlags::PerfFilter) || \
//							(((TFlags)&EAutomationTestFlags::FilterMask) == EAutomationTestFlags::StressFilter) || \
//							(((TFlags)&EAutomationTestFlags::FilterMask) == EAutomationTestFlags::NegativeFilter), \
//							"All AutomationTests must have exactly 1 filter type specified.  See AutomationTest.h."); \
//		} \
//		virtual uint32 GetTestFlags() const override { return TFlags; } \
//		virtual bool IsStressTest() const { return false; } \
//		virtual uint32 GetRequiredDeviceNum() const override { return 1; } \
//		virtual FString GetTestSourceFileName() const override { return FileName; } \
//		virtual int32 GetTestSourceFileLine() const override { return LineNumber; } \
//	protected: \
//		virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray <FString>& OutTestCommands) const override \
//		{ \
//			OutBeautifiedNames.Add(PrettyName); \
//			OutTestCommands.Add(FString()); \
//		} \
//		virtual bool RunTest(const FString& Parameters) override; \
//		virtual FString GetBeautifiedTestName() const override { return PrettyName; } \
//	private: \
//		void Define(); \
//	};
//
//#if WITH_AUTOMATION_WORKER
//#define IMPLEMENT_SIMPLE_SPATIAL_AUTOMATION_TEST( TClass, PrettyName, TFlags ) \
//		IMPLEMENT_SIMPLE_SPATIAL_AUTOMATION_TEST_PRIVATE(TClass, PrettyName, TFlags, __FILE__, __LINE__) \
//		namespace\
//		{\
//			TClass TClass##AutomationTestInstance( TEXT(#TClass) );\
//		}
//#else
//#define IMPLEMENT_SIMPLE_SPATIAL_AUTOMATION_TEST( TClass, PrettyName, TFlags ) \
//		IMPLEMENT_SIMPLE_SPATIAL_AUTOMATION_TEST_PRIVATE(TClass, PrettyName, TFlags, __FILE__, __LINE__)
//#endif // #if WITH_AUTOMATION_WORKER
//
