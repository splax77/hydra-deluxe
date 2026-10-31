// Clone Hero v1.0.0.4045 (Mono build), decompiled with ilspycmd 11.0.0.9375 from
// C:\Clone Hero\1.0 old\Clone Hero\Clone Hero_Data\Managed\Assembly-CSharp.dll on 2026-10-30.
// Only the relevant methods are kept, copied unchanged. Names are the game's scrambled names.
// Line numbers in each banner are lines of the full decompiled file, not of this file.

// ---- caller header: static class ʶˀʵˁʹʿˀʶʿʲʾ method ʹʹʾʴʳʶʼʵʳʼʼ (start) : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 823 to 833 of the ilspycmd output ----
	public static ʹʲʼʺʶʷʾʶʿʺʹ ʹʹʾʴʳʶʼʵʳʼʼ(BasePlayer ʾʵʻʳʷʴʼʲʻʶʷ, ʷʽʻʻʿʵˀˁʽʳʶ ʴʼʿʼʺʹʸʲʽʾʽ, bool ʷʻʺʽʻˀʾʸʹʸʲ = true)
	{
		Instrument instrument = ʾʵʻʳʷʴʼʲʻʶʷ.ʾʵʻʳʷʴʼʲʻʶʷ.ʲʼʽʺʿʹʽʻʵʺʶ.instrument;
		ControllerType getControllerType = ʾʵʻʳʷʴʼʲʻʶʷ.ʾʵʻʳʷʴʼʲʻʶʷ.ʲʼʽʺʿʹʽʻʵʺʶ.controllerType.GetControllerType;
		ʹʺʿʻʲʹʻʹʹʵʸ ʲʼʽʺʿʹʽʻʵʺʶ = ʾʵʻʳʷʴʼʲʻʶʷ.ʾʵʻʳʷʴʼʲʻʶʷ.ʲʼʽʺʿʹʽʻʵʺʶ;
		ʷʽʻʻʿʵˀˁʽʳʶ.ʼˀʼʳʴˁʳʾʲˁʾ ʷʻʺʼʴʶʴʳʽʺʿ = ʲʼʽʺʿʹʽʻʵʺʶ.ʺʽʼʼʴʽʴʾʲʷˁ;
		if (ʲʼʽʺʿʹʽʻʵʺʶ.ʺʽʼʼʴʽʴʾʲʷˁ == ʷʽʻʻʿʵˀˁʽʳʶ.ʼˀʼʳʴˁʳʾʲˁʾ.ProDrums)
		{
			ʷʻʺʼʴʶʴʳʽʺʿ = ʷʽʻʻʿʵˀˁʽʳʶ.ʼˀʼʳʴˁʳʾʲˁʾ.Drums;
		}
		ʹʲʼʺʶʷʾʶʿʺʹ result = default(ʹʲʼʺʶʷʾʶʿʺʹ);

// ---- caller choice inside ʹʹʾʴʳʶʼʵʳʼʼ: generate only when the track has no written fills : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 884 to 891 of the ilspycmd output ----
			if (ˀʺʷʺʵʾʼʿʹʼʸ2.ʸʶʳʸʸʲʶʺʼʽʳ.Count == 0)
			{
				result.ʲʻʳʸʵʲˀʿʺʴʼ = ʸˀʽʸʽʳʳʸʼʾʿ(result.ˀʶʵʾʻʹʲʿˀʻˀ, ˀʺʷʺʵʾʼʿʹʼʸ2);
			}
			else
			{
				result.ʲʻʳʸʵʲˀʿʺʴʼ = ʳʷˁʸʵʾʼʷˀʲˁ(result.ˀʶʵʾʻʹʲʿˀʻˀ, ʴʼʿʼʺʹʸʲʽʾʽ, ˀʺʷʺʵʾʼʿʹʼʸ2);
			}

// ---- F1 fill generator ʸˀʽʸʽʳʳʸʼʾʿ : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 178 to 225 of the ilspycmd output ----
	public static List<ʲʺʲʴʶʴʶʻʺʷʴ> ʸˀʽʸʽʳʳʸʼʾʿ(List<ʼˀˀʸʸʶʳʼʳʸʶ> ʲʿʺʾʻʵˁʽʻʴʸ, ˀʺʷʺʵʾʼʿʹʼʸ ʴˁʶʳʳʹʽʴʳʽʵ)
	{
		List<float> ˁʹʾʲʸʿʲʷʻˁʽ = ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ʵʹˁˀʷʽʾʲˁʲʶ.ˁʹʾʲʸʿʲʷʻˁʽ;
		List<ʲʺʲʴʶʴʶʻʺʷʴ> list = new List<ʲʺʲʴʶʴʶʻʺʷʴ>();
		if (ʲʿʺʾʻʵˁʽʻʴʸ != null && ʲʿʺʾʻʵˁʽʻʴʸ.Count == 0)
		{
			return list;
		}
		int ʻʾˁʽʺˀʽʺʻʼʺ = 0;
		uint num = (uint)Mathf.RoundToInt(12f * ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ˀʳˁʻʵʺʻʳˁʸʽ / ʵʷʽʴʹʽʳʹʷʴʷ.ˀˀʽʵʶʸʸʿʷʾʲ);
		for (int i = 1; i < ˁʹʾʲʸʿʲʷʻˁʽ.Count; i += 4)
		{
			float num2 = ˁʹʾʲʸʿʲʷʻˁʽ[i - 1] + (ˁʹʾʲʸʿʲʷʻˁʽ[i] - ˁʹʾʲʸʿʲʷʻˁʽ[i - 1]) / 2f;
			float num3 = ˁʹʾʲʸʿʲʷʻˁʽ[i];
			int num4 = ʲʿʺʾʻʵˁʽʻʴʸ.ʶʲˁʿʹʵˀʺʲˁʿ(ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ʵʲˁˀʲʳʼʶʾˀʺ(num3), ʻʾˁʽʺˀʽʺʻʼʺ);
			if (num4 == -1)
			{
				i -= 3;
				continue;
			}
			ʼˀˀʸʸʶʳʼʳʸʶ ʼˀˀʸʸʶʳʼʳʸʶ2 = ʲʿʺʾʻʵˁʽʻʴʸ[num4];
			float num5 = ʼˀˀʸʸʶʳʼʳʸʶ2.ʲʸʺʺʼʷʵʵʶʵˁ - num3;
			num5 = ((num5 < 0f) ? (0f - num5) : num5);
			if (num5 > 0.25f)
			{
				i -= 3;
				continue;
			}
			ʻʾˁʽʺˀʽʺʻʼʺ = num4;
			ʼˀˀʸʸʶʳʼʳʸʶ ʼˀˀʸʸʶʳʼʳʸʶ3 = ʿʹʽʽʵʻʺʸʻʵʳ(ʼˀˀʸʸʶʳʼʳʸʶ2, ʾʲʴʵˀʷʸʺʾʹʷ: false, ʴʸʽʹʹʳʾʲʹˁʼ: true);
			if (ʼˀˀʸʸʶʳʼʳʸʶ3 != null)
			{
				ʼˀˀʸʸʶʳʼʳʸʶ3.ˀʷˁʵʹʳʿʵʼʹˀ |= ʼˀˀʸʸʶʳʼʳʸʶ.ʴʴʶʹʼʳʽʵʴʳʿ.DRUM_ACTIVATION;
				uint num6 = ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ʵʲˁˀʲʳʼʶʾˀʺ(num2);
				ʲʺʲʴʶʴʶʻʺʷʴ item = new ʲʺʲʴʶʴʶʻʺʷʴ
				{
					ʲʸʺʺʼʷʵʵʶʵˁ = num2,
					ʻʾʺʸʾʹʿʳʴʸʽ = ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ʸʳʾʸʼʹʽʾʿʸʵ(num6, ʼˀˀʸʸʶʳʼʳʸʶ3.ʵʿʵʲʽʽʽʵʸˀʾ + num),
					ʹʾʽʿʻʻʾʿʼʳʸ = ʼˀˀʸʸʶʳʼʳʸʶ3,
					ʵʿʵʲʽʽʽʵʸˀʾ = num6,
					ʷʳʽʵʳʴʼʲʴˁʲ = (int)(ʼˀˀʸʸʶʳʼʳʸʶ3.ʵʿʵʲʽʽʽʵʸˀʾ + num - num6),
					ʵˀʺʾʼʿʷʼʲʲʷ = true
				};
				list.Add(item);
			}
		}
		return list;
	}

// ---- F2 written-fill placement ʳʷˁʸʵʾʼʷˀʲˁ : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 467 to 501 of the ilspycmd output ----
	public static List<ʲʺʲʴʶʴʶʻʺʷʴ> ʳʷˁʸʵʾʼʷˀʲˁ(List<ʼˀˀʸʸʶʳʼʳʸʶ> ʲʿʺʾʻʵˁʽʻʴʸ, ʷʽʻʻʿʵˀˁʽʳʶ ʴʼʿʼʺʹʸʲʽʾʽ, ˀʺʷʺʵʾʼʿʹʼʸ ʴˁʶʳʳʹʽʴʳʽʵ)
	{
		_ = ʲʿʺʾʻʵˁʽʻʴʸ.Count;
		List<ʲʺʲʴʶʴʶʻʺʷʴ> list = new List<ʲʺʲʴʶʴʶʻʺʷʴ>(ʴˁʶʳʳʹʽʴʳʽʵ.ʸʶʳʸʸʲʶʺʼʽʳ.Count);
		uint num = (uint)Mathf.RoundToInt(6f * ʴʼʿʼʺʹʸʲʽʾʽ.ˀʳˁʻʵʺʻʳˁʸʽ / ʵʷʽʴʹʽʳʹʷʴʷ.ˀˀʽʵʶʸʸʿʷʾʲ);
		uint num2 = (uint)Mathf.RoundToInt(12f * ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ˀʳˁʻʵʺʻʳˁʸʽ / ʵʷʽʴʹʽʳʹʷʴʷ.ˀˀʽʵʶʸʸʿʷʾʲ);
		foreach (ʳʹʺˁʲʵʳˀʸʿʶ item2 in ʴˁʶʳʳʹʽʴʳʽʵ.ʸʶʳʸʸʲʶʺʼʽʳ)
		{
			if (ʴʼʿʼʺʹʸʲʽʾʽ.ʾʹʷˀʼʼˁʽʼʽʾ != null && ʴʼʿʼʺʹʸʲʽʾʽ.ʾʹʷˀʼʼˁʽʼʽʾ.ʳʿʹʺʵʹʷʼʼʸʸ == item2.ʳʿʹʺʵʹʷʼʼʸʸ)
			{
				continue;
			}
			uint num3 = item2.ʳʿʹʺʵʹʷʼʼʸʸ + item2.ʽʹʸʲˁʴʸˀʳʻʼ;
			ʼˀˀʸʸʶʳʼʳʸʶ ʼˀˀʸʸʶʳʼʳʸʶ2 = ʲʿʺʾʻʵˁʽʻʴʸ.ʷʾˀʼˀʵʹˁʻʻʷ(num3, item2.ʳʿʹʺʵʹʷʼʼʸʸ, num3 + num);
			if (ʼˀˀʸʸʶʳʼʳʸʶ2 != null)
			{
				ʼˀˀʸʸʶʳʼʳʸʶ ʼˀˀʸʸʶʳʼʳʸʶ3 = ʿʹʽʽʵʻʺʸʻʵʳ(ʼˀˀʸʸʶʳʼʳʸʶ2, ʾʲʴʵˀʷʸʺʾʹʷ: false, ʴʸʽʹʹʳʾʲʹˁʼ: true);
				if (ʼˀˀʸʸʶʳʼʳʸʶ3 != null)
				{
					ʼˀˀʸʸʶʳʼʳʸʶ3.ˀʷˁʵʹʳʿʵʼʹˀ |= ʼˀˀʸʸʶʳʼʳʸʶ.ʴʴʶʹʼʳʽʵʴʳʿ.DRUM_ACTIVATION;
					ʲʺʲʴʶʴʶʻʺʷʴ item = new ʲʺʲʴʶʴʶʻʺʷʴ
					{
						ʲʸʺʺʼʷʵʵʶʵˁ = item2.ʲʸʺʺʼʷʵʵʶʵˁ,
						ʻʾʺʸʾʹʿʳʴʸʽ = ʴˁʶʳʳʹʽʴʳʽʵ.ʷʳʷʻʸʽʽʾˁʽʵ.ʸʳʾʸʼʹʽʾʿʸʵ(item2.ʳʿʹʺʵʹʷʼʼʸʸ, ʼˀˀʸʸʶʳʼʳʸʶ3.ʵʿʵʲʽʽʽʵʸˀʾ + num2),
						ʹʾʽʿʻʻʾʿʼʳʸ = ʼˀˀʸʸʶʳʼʳʸʶ3,
						ʵʿʵʲʽʽʽʵʸˀʾ = item2.ʳʿʹʺʵʹʷʼʼʸʸ,
						ʷʳʽʵʳʴʼʲʴˁʲ = (int)(ʼˀˀʸʸʶʳʼʳʸʶ3.ʵʿʵʲʽʽʽʵʸˀʾ - item2.ʳʿʹʺʵʹʷʼʼʸʸ + num2),
						ʵˀʺʾʼʿʷʼʲʲʷ = true
					};
					list.Add(item);
				}
			}
		}
		return list;
	}

// ---- F3 pick the activation note of a chord ʶʸʲʷʵʸʻʻʸʹʸ : ʶˀʵˁʹʿˀʶʿʲʾ.cs lines 1017 to 1041 of the ilspycmd output ----
	private static ʼˀˀʸʸʶʳʼʳʸʶ ʶʸʲʷʵʸʻʻʸʹʸ(ʼˀˀʸʸʶʳʼʳʸʶ ʹʿʶʾʺʾˁʴʻˁʲ, bool ʴʸʽʹʹʳʾʲʹˁʼ = false)
	{
		ushort num = ʹʿʶʾʺʾˁʴʻˁʲ.ʵʹʴʸʽʷʺʳʵʿʳ;
		uint num2 = (uint)(num & 0x7F);
		uint num3 = (uint)(num & 0x180);
		uint num4 = 0u;
		if (num2 != 0)
		{
			num4 = ʵʼʴʿʼʷʷˀʶˁʷ.ʷʹʾˁʳʸʲʾʴʾʿ(num2);
		}
		else if (ʴʸʽʹʹʳʾʲʹˁʼ && num3 != 0)
		{
			num4 = ʵʼʴʿʼʷʷˀʶˁʷ.ʴʽʸʿʼʸˁʷʿʹʸ(num3);
		}
		ʼˀˀʸʸʶʳʼʳʸʶ.ˀˁˀʼʵʼˀʵʷʸʳ ˀˁˀʼʵʼˀʵʷʸʳ = ʹʿʶʾʺʾˁʴʻˁʲ.ʾʽʶʻʸʾʳʴʿʳʲ.ʼˀʸʹʴʻʾʶʼʷˁ();
		while (ˀˁˀʼʵʼˀʵʷʸʳ.ʿʵʷʻʵʻʷʵʶʺʼ())
		{
			ʼˀˀʸʸʶʳʼʳʸʶ ʼˀˀʸʸʶʳʼʳʸʶ2 = ˀˁˀʼʵʼˀʵʷʸʳ.ʹʺʻʴˀʴʶʲʹʳʿ;
			if (ʼˀˀʸʸʶʳʼʳʸʶ2.ʿˀʸʻʺʸʳʺʴʻʿ == num4)
			{
				return ʼˀˀʸʸʶʳʼʳʸʶ2;
			}
		}
		return null;
	}

// ---- F4 nearest chord to a tick ʶʲˁʿʹʵˀʺʲˁʿ : ʴʻˁʷʸʶʶʶʿʳʹ.cs lines 194 to 217 of the ilspycmd output ----
	public static int ʶʲˁʿʹʵˀʺʲˁʿ<T>(this IList<T> ʹʺʼʿʲʸʹʺʲʶʹ, uint ʵʻʼˀʵˁʿʽʵʷʹ, int ʻʾˁʽʺˀʽʺʻʼʺ = 0) where T : class, ʶʼˀˁʲʴʿʾʵʲʻ
	{
		int num = ʹʺʼʿʲʸʹʺʲʶʹ.ʼʿˀʹʵˁˁʹʽʸʹ(ʵʻʼˀʵˁʿʽʵʷʹ, ʻʾˁʽʺˀʽʺʻʼʺ);
		int ʻʾˁʽʺˀʽʺʻʼʺ2 = ((num == -1) ? ʻʾˁʽʺˀʽʺʻʼʺ : num);
		int num2 = ʹʺʼʿʲʸʹʺʲʶʹ.ʴʶʾˀˁʾʵʼʾʺʽ(ʵʻʼˀʵˁʿʽʵʷʹ, ʻʾˁʽʺˀʽʺʻʼʺ2, ʴʼʲʹʾʵʾʹʼʵʿ: true);
		if (num2 == -1 && num == -1)
		{
			return -1;
		}
		if (num == -1)
		{
			return num2;
		}
		if (num2 == -1)
		{
			return num;
		}
		T val = ʹʺʼʿʲʸʹʺʲʶʹ[num2];
		if (ʹʺʼʿʲʸʹʺʲʶʹ[num].ʵʿʵʲʽʽʽʵʸˀʾ - ʵʻʼˀʵˁʿʽʵʷʹ > ʵʻʼˀʵˁʿʽʵʷʹ - val.ʵʿʵʲʽʽʽʵʸˀʾ)
		{
			return num2;
		}
		return num;
	}

// ---- F4 helper: first chord strictly after a tick ʼʿˀʹʵˁˁʹʽʸʹ : ʴʻˁʷʸʶʶʶʿʳʹ.cs lines 6 to 17 of the ilspycmd output ----
	public static int ʼʿˀʹʵˁˁʹʽʸʹ<T>(this IList<T> ʹʺʼʿʲʸʹʺʲʶʹ, uint ʵʻʼˀʵˁʿʽʵʷʹ, int ʻʾˁʽʺˀʽʺʻʼʺ = 0) where T : class, ʶʼˀˁʲʴʿʾʵʲʻ
	{
		foreach (var item in EnumerableRanges.EnumerateRangeIndexed(ʹʺʼʿʲʸʹʺʲʶʹ, ʻʾˁʽʺˀʽʺʻʼʺ))
		{
			var (result, _) = item;
			if (item.Item2.ʵʿʵʲʽʽʽʵʸˀʾ > ʵʻʼˀʵˁʿʽʵʷʹ)
			{
				return result;
			}
		}
		return 0;
	}

// ---- F4 helper: last chord at or before a tick ʴʶʾˀˁʾʵʼʾʺʽ : ʴʻˁʷʸʶʶʶʿʳʹ.cs lines 140 to 150 of the ilspycmd output ----
	public static int ʴʶʾˀˁʾʵʼʾʺʽ<T>(this IList<T> ʹʺʼʿʲʸʹʺʲʶʹ, uint ʵʻʼˀʵˁʿʽʵʷʹ, int ʻʾˁʽʺˀʽʺʻʼʺ = int.MaxValue, bool ʴʼʲʹʾʵʾʹʼʵʿ = false) where T : class, ʶʼˀˁʲʴʿʾʵʲʻ
	{
		foreach (var (result, val) in EnumerableRanges.EnumerateRangeReversedIndexed(ʹʺʼʿʲʸʹʺʲʶʹ, ʻʾˁʽʺˀʽʺʻʼʺ))
		{
			if (val.ʵʿʵʲʽʽʽʵʸˀʾ < ʵʻʼˀʵˁʿʽʵʷʹ || ((val.ʵʿʵʲʽʽʽʵʸˀʾ <= ʵʻʼˀʵˁʿʽʵʷʹ) & ʴʼʲʹʾʵʾʹʼʵʿ))
			{
				return result;
			}
		}
		return -1;
	}

// ---- F5 measure-line and beat list builder ʴˀʲʸʷʶˁˀʺʶʲ (class ʵʹˁˀʷʽʾʲˁʲʶ) : ʵʹˁˀʷʽʾʲˁʲʶ.cs lines 52 to 110 of the ilspycmd output ----
	public void ʴˀʲʸʷʶˁˀʺʶʲ(ʷʽʻʻʿʵˀˁʽʳʶ ʴʼʿʼʺʹʸʲʽʾʽ)
	{
		uint num = 0u;
		uint num2 = ʴʼʿʼʺʹʸʲʽʾʽ.ʿʼʿʿʵʵʲʿʷˁʻ();
		ˀʸʼʹʽʺʸˀʷʽʵ ˀʸʼʹʽʺʸˀʷʽʵ2 = null;
		ˀʸʼʹʽʺʸˀʷʽʵ ˀʸʼʹʽʺʸˀʷʽʵ3 = ʴʼʿʼʺʹʸʲʽʾʽ.ʴʵʿʹʹʾʿʳˀʵʼ(num);
		int num3 = 0;
		int num4 = 0;
		while (num <= num2)
		{
			ˀʸʼʹʽʺʸˀʷʽʵ3 = ʴʼʿʼʺʹʸʲʽʾʽ.ʴʵʿʹʹʾʿʳˀʵʼ(num);
			uint num5 = ˀʸʼʹʽʺʸˀʷʽʵ3.ʲʲˀʵʼʿʳʸʾʾʴ / 4;
			bool flag = ˀʸʼʹʽʺʸˀʷʽʵ2 == null || (ˀʸʼʹʽʺʸˀʷʽʵ2 != null && (ˀʸʼʹʽʺʸˀʷʽʵ2.ʶʶˁʿʷʾʹʸʴʷʲ != ˀʸʼʹʽʺʸˀʷʽʵ3.ʶʶˁʿʷʾʹʸʴʷʲ || ˀʸʼʹʽʺʸˀʷʽʵ2.ʲʲˀʵʼʿʳʸʾʾʴ != ˀʸʼʹʽʺʸˀʷʽʵ3.ʲʲˀʵʼʿʳʸʾʾʴ));
			if (flag)
			{
				num4 = 0;
				num3 = 0;
			}
			if (num3 == ˀʸʼʹʽʺʸˀʷʽʵ3.ʶʶˁʿʷʾʹʸʴʷʲ)
			{
				num3 = 0;
			}
			if ((num3 == 0) | flag)
			{
				if (ˀʸʼʹʽʺʸˀʷʽʵ3.ʶʶˁʿʷʾʹʸʴʷʲ == 1 && num4 > 0)
				{
					if (ˀʸʼʹʽʺʸˀʷʽʵ3.ʲʲˀʵʼʿʳʸʾʾʴ <= 4 || num4 % num5 == 0L)
					{
						ʼʺʾʼˁʼʹˁˁʷʻ(ʴʼʿʼʺʹʸʲʽʾʽ, num, ʻʴʴʸʾʹˁʸʺʽʷ.BEAT_STRONG);
					}
					else
					{
						ʼʺʾʼˁʼʹˁˁʷʻ(ʴʼʿʼʺʹʸʲʽʾʽ, num, ʻʴʴʸʾʹˁʸʺʽʷ.BEAT_WEAK);
					}
				}
				else
				{
					if (ˀʸʼʹʽʺʸˀʷʽʵ2 != null && ˀʸʼʹʽʺʸˀʷʽʵ2.ʲʲˀʵʼʿʳʸʾʾʴ > 4)
					{
						ʶʺʾʺʾʹʻʿʽʽʸ[ʶʺʾʺʾʹʻʿʽʽʸ.Count - 1] = ʻʴʴʸʾʹˁʸʺʽʷ.BEAT_WEAK;
					}
					ʼʺʾʼˁʼʹˁˁʷʻ(ʴʼʿʼʺʹʸʲʽʾʽ, num, ʻʴʴʸʾʹˁʸʺʽʷ.MEASURE);
				}
			}
			else if (ˀʸʼʹʽʺʸˀʷʽʵ3.ʲʲˀʵʼʿʳʸʾʾʴ <= 4 || num3 % num5 == 0L)
			{
				ʼʺʾʼˁʼʹˁˁʷʻ(ʴʼʿʼʺʹʸʲʽʾʽ, num, ʻʴʴʸʾʹˁʸʺʽʷ.BEAT_STRONG);
			}
			else
			{
				ʼʺʾʼˁʼʹˁˁʷʻ(ʴʼʿʼʺʹʸʲʽʾʽ, num, ʻʴʴʸʾʹˁʸʺʽʷ.BEAT_WEAK);
			}
			num3++;
			num4++;
			num += (uint)(ʴʼʿʼʺʹʸʲʽʾʽ.ˀʳˁʻʵʺʻʳˁʸʽ * 4f / (float)ˀʸʼʹʽʺʸˀʷʽʵ3.ʲʲˀʵʼʿʳʸʾʾʴ);
			ˀʸʼʹʽʺʸˀʷʽʵ2 = ˀʸʼʹʽʺʸˀʷʽʵ3;
		}
		ʾʾʼʴˀʲʺʺʵʸʵ = ˁʹʾʲʸʿʲʷʻˁʽ.Count - 1;
	}

